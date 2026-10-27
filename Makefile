CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Iinclude
SRC = src/graph.c src/heap.c src/coloring.c src/room_allocation.c src/validator.c src/test_suite.c src/main.c
OUT = bin/exam_scheduler.exe

all: $(OUT)

$(OUT): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

test: $(OUT)
	./$(OUT) --test

run: $(OUT)
	./$(OUT) --run

benchmark: $(OUT)
	./$(OUT) --benchmark

clean:
	rm -rf bin/*.exe bin/*.o
