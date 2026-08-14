CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb

.PHONY: all test coverage clean

all: heap

heap: main.c heap.c heap.h
	$(CC) $(CFLAGS) -o heap main.c heap.c

test: test.c heap.c heap.h
	$(CC) $(CFLAGS) -o test_heap test.c heap.c
	./test_heap

coverage: test.c heap.c heap.h
	$(CC) $(CFLAGS) --coverage -o test_heap test.c heap.c
	./test_heap
	gcov -b test_heap-heap.gcno

clean:
	rm -f heap test_heap *.o *.gcda *.gcno *.gcov
