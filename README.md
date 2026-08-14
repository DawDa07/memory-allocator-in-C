# Memory allocator in C

A first-fit heap over a fixed 640KB arena, with coalescing, 16-byte aligned allocations, `realloc`, and a conservative mark-and-sweep garbage collector.

## Build

```bash
make          # linked-list + GC demo
make test     # unit tests
make stress   # deterministic alloc/free fragmentation run
make coverage # gcov line coverage of heap.c
```

## How it works

Allocations come from a static `heap` array. Two address-sorted lists track regions:

- **allocated** — live chunks returned to the program
- **free** — holes that can be reused

**`heap_alloc`** coalesces adjacent free chunks, then takes the first hole that fits. Leftover bytes stay on the free list. Sizes are rounded up to 16 bytes so returned pointers stay aligned.

**`heap_free`** moves a chunk back to the free list. Neighbors are merged on the next alloc (or realloc).

**`heap_realloc`** grows in place when the following free hole is large enough; otherwise it allocates, copies, and frees. Shrinking returns the tail to the free list.

**`heap_collect`** is a conservative GC. After `heap_init` records the top of the C stack, collect scans stack words and the payloads of already-marked chunks. Any bit pattern that looks like a pointer into an allocated chunk keeps that chunk. Unmarked chunks are swept onto the free list.

```
alloced: [A][B][C]
freed:   [----hole----]

heap_free(B) -> hole between A and C
next alloc coalesces if A/C are also freed, or first-fits the hole
```

## Demo

`make` builds a list of nodes 1..20 on this heap, then cuts the chain after 5 and runs the GC. You should see 20 allocated chunks before collect and 5 after, plus `heap_stats` (bytes in use, free-list holes, largest free chunk).

## Limits

- Arena is 640KB and at most 1024 chunks; there is no `mmap` growth
- GC is conservative: leftover stack words can keep objects alive; objects are never moved
- Not a replacement for libc `malloc`
- Call `heap_init(__builtin_frame_address(0))` from `main` before `heap_collect`
