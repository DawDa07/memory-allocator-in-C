CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
DS_SRCS=ds/vec.c ds/list.c ds/hashmap.c
HEAP_SRC=heap.c

.PHONY: all test test_ds demos stress coverage clean

all: heap

heap: demos/list_gc.c $(HEAP_SRC) heap.h ds/list.c ds/list.h
	$(CC) $(CFLAGS) -o heap demos/list_gc.c ds/list.c $(HEAP_SRC)

demos: heap vec_demo

vec_demo: demos/vec_demo.c $(HEAP_SRC) heap.h ds/vec.c ds/vec.h
	$(CC) $(CFLAGS) -o vec_demo demos/vec_demo.c ds/vec.c $(HEAP_SRC)

test: test_heap.c $(HEAP_SRC) heap.h
	$(CC) $(CFLAGS) -o test_heap test_heap.c $(HEAP_SRC)
	./test_heap

test_ds: test_ds.c $(HEAP_SRC) heap.h $(DS_SRCS) ds/vec.h ds/list.h ds/hashmap.h
	$(CC) $(CFLAGS) -o test_ds test_ds.c $(DS_SRCS) $(HEAP_SRC)
	./test_ds

stress: stress.c $(HEAP_SRC) heap.h
	$(CC) $(CFLAGS) -o heap_stress stress.c $(HEAP_SRC)
	./heap_stress

coverage: test_heap.c $(HEAP_SRC) heap.h
	rm -f *.gcda *.gcno *.gcov
	$(CC) $(CFLAGS) --coverage -o test_heap test_heap.c $(HEAP_SRC)
	./test_heap
	gcov -b test_heap-heap.gcno

clean:
	rm -f heap vec_demo test_heap test_ds heap_stress *.o *.gcda *.gcno *.gcov
