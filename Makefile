CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb -Isrc -Ids
HEAP_SRC=src/heap.c
DS_SRCS=ds/vec.c ds/list.c ds/hashmap.c
BUILD=build

.PHONY: all test test_ds demos stress coverage clean

all: $(BUILD)/heap

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/heap: demos/list_gc.c $(HEAP_SRC) src/heap.h ds/list.c ds/list.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ demos/list_gc.c ds/list.c $(HEAP_SRC)

$(BUILD)/vec_demo: demos/vec_demo.c $(HEAP_SRC) src/heap.h ds/vec.c ds/vec.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ demos/vec_demo.c ds/vec.c $(HEAP_SRC)

demos: $(BUILD)/heap $(BUILD)/vec_demo

test: $(BUILD)/test_heap
	./$(BUILD)/test_heap

$(BUILD)/test_heap: tests/test_heap.c $(HEAP_SRC) src/heap.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/test_heap.c $(HEAP_SRC)

test_ds: $(BUILD)/test_ds
	./$(BUILD)/test_ds

$(BUILD)/test_ds: tests/test_ds.c $(HEAP_SRC) src/heap.h $(DS_SRCS) ds/vec.h ds/list.h ds/hashmap.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/test_ds.c $(DS_SRCS) $(HEAP_SRC)

stress: $(BUILD)/heap_stress
	./$(BUILD)/heap_stress

$(BUILD)/heap_stress: tests/stress.c $(HEAP_SRC) src/heap.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ tests/stress.c $(HEAP_SRC)

coverage: tests/test_heap.c $(HEAP_SRC) src/heap.h | $(BUILD)
	rm -f $(BUILD)/*.gcda $(BUILD)/*.gcno *.gcov
	$(CC) $(CFLAGS) --coverage -o $(BUILD)/test_heap tests/test_heap.c $(HEAP_SRC)
	./$(BUILD)/test_heap
	gcov -b -o $(BUILD) $(HEAP_SRC)

clean:
	rm -rf $(BUILD) *.gcov *.dSYM
