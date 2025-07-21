CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

allocator:
	@$(CC) -c src/allocator.c $(CFLAGS) -o bin/allocator.o
	@echo "Built allocator.o"

test:
	@$(CC) test/test_allocator.c bin/allocator.o $(CFLAGS) -o bin/test_allocator
	@./bi/test_allocator
	@echo "Finished running test"

build: allocator test

clean: 
	rm -rf bin

