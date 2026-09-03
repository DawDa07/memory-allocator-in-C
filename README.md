# Memory allocator in C

A first-fit heap over a fixed 640KB arena, with coalescing, 16-byte aligned allocations, `realloc`, and a conservative mark-and-sweep garbage collector.

## Layout

```
src/       allocator (heap.c / heap.h)
ds/        containers on top of the heap
demos/     list GC + vector demos
tests/     allocator, ds, and stress tests
build/     compiled binaries
```

## Build

```bash
make          # linked-list + GC demo → build/heap
make demos    # list GC, vector, and list demos
make test     # allocator unit tests
make test_ds  # vector / list / hashmap tests
make stress   # deterministic alloc/free fragmentation run
make coverage # gcov line coverage of heap.c
```

## How it works

Allocations come from a static `heap` array. Two address-sorted lists track regions:

- **allocated** — live chunks returned to the program
- **free** — holes that can be reused

Sizes are rounded up to 16 bytes. Letters are in use; `.` is free.

### `heap_alloc` — first-fit, then split

Start with one free region covering the whole arena:

```
heap:  [................................]
alloc: (empty)
free:  [all]
```

`heap_alloc(32)` takes the first hole that fits and leaves the tail free:

```
heap:  [AAAA............................]
alloc: [A=32]
free:       [tail]
```

`heap_alloc(32)` again:

```
heap:  [AAAABBBB........................]
alloc: [A][B]
free:           [tail]
```

### `heap_free` — hole in the middle

`heap_free(A)` does **not** merge yet. A becomes a hole; B stays allocated:

```
heap:  [....BBBB........................]
alloc:     [B]
free:  [A]      [tail]
```

A later `heap_alloc(48)` **skips** the 32-byte hole (too small) and takes from the tail (first-fit):

```
heap:  [....BBBBCCCCCCCC................]
alloc:     [B][C=48]
free:  [A]              [tail]
```

### Coalesce — merge on the next alloc

If you free B as well, two neighbor holes sit side by side. The next `heap_alloc` merges them before searching:

```
after free(B):
heap:  [................................]
free:  [A][B][tail]    (not merged yet)

next alloc coalesces first:
free:  [entire arena again]
```

Then a request like `heap_alloc(60)` can reuse the combined space at the start.

### `heap_realloc`

Grow **in place** when the bytes after the block are free:

```
[AAAA........]  realloc(A, 48)  ->  [AAAAAAA...]   same pointer
```

Grow **by move** when the next region is still allocated:

```
[AAAABBBB....]  realloc(A, 64)  ->  [....BBBBAAAA]  copy, then free old A
```

Shrink gives the tail back to the free list:

```
[AAAAAAAA....]  realloc(A, 16)  ->  [AA..........]
```

### `heap_collect` — mark from the stack, then sweep

After `heap_init`, collect treats stack words (and bytes inside already-marked objects) as possible pointers.

Linked-list demo: nodes `1 -> 2 -> 3 -> 4 -> 5 -> 6 -> ... -> 20`, then cut after 5:

```
stack:  head -----------------.
                              v
heap:  [1]->[2]->[3]->[4]->[5]  [6]->[7]->...->[20]
                                 ^
                                 no root points here
```

Mark follows `head` and each `next` pointer. Sweep frees 6..20:

```
stack:  head -----------------.
                              v
heap:  [1]->[2]->[3]->[4]->[5]  [............free............]
```

It is **conservative**: it does not know types. If a leftover integer on the stack looks like an address inside `[6]`, that node (and anything it points to) can stay alive. Objects are never moved.

## Demo

`make` builds a list of nodes 1..20 on this heap (via `ds/list`), then cuts the chain after 5 and runs the GC. You should see 20 allocated chunks before collect and 5 after, plus `heap_stats` (bytes in use, free-list holes, largest free chunk).

`make demos` also builds `vec_demo`, which grows one contiguous buffer with `heap_realloc` so 20 ints use a single allocated chunk.

## Data structures (`ds/`)

Thin containers that allocate only through `heap_alloc` / `heap_free` / `heap_realloc`:

| Module | Role |
|--------|------|
| `ds/vec` | Growable `int` array (doubling via `realloc`) |
| `ds/list` | Singly linked list of `int` (head + tail; push/pop, index ops, reverse) |
| `ds/hashmap` | Open-addressing `int → int` map (one slot table) |

Normal use calls `vec_free` / `list_free` / `hashmap_free`. The list GC demo is the exception: it drops reachability and relies on `heap_collect`. Prefer contiguous structures (vector, hashmap table) when you care about the 1024-chunk cap — many tiny list nodes consume one chunk each.

## Limits

- Arena is 640KB and at most 1024 chunks; there is no `mmap` growth
- GC is conservative: leftover stack words can keep objects alive; objects are never moved
- Not a replacement for libc `malloc`
- Call `heap_init(__builtin_frame_address(0))` from `main` before `heap_collect`
