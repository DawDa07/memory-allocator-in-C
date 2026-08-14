CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb

.PHONY: all test stress coverage clean

all: heap

heap: main.c heap.c heap.h
	$(CC) $(CFLAGS) -o heap main.c heap.c

test: test.c heap.c heap.h
	$(CC) $(CFLAGS) -o test_heap test.c heap.c
	./test_heap

stress: stress.c heap.c heap.h
	$(CC) $(CFLAGS) -o heap_stress stress.c heap.c
	./heap_stress

coverage: test.c heap.c heap.h
	rm -f *.gcda *.gcno *.gcov
	$(CC) $(CFLAGS) --coverage -o test_heap test.c heap.c
	./test_heap
	gcov -b test_heap-heap.gcno

clean:
	rm -f heap test_heap heap_stress *.o *.gcda *.gcno *.gcov
