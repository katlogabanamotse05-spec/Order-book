CC   = cc
WARN = -Wall -Wextra -Wpedantic -std=c11
SRC  = $(wildcard src/*.c)

all: order_book

order_book: $(SRC)
	$(CC) $(WARN) -O2 -Iinclude $(SRC) -o order_book

debug: $(SRC)
	$(CC) $(WARN) -O0 -g -Iinclude $(SRC) -o order_book_debug

asan: $(SRC)
	$(CC) $(WARN) -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude $(SRC) -o order_book_asan

test: asan
	@echo "no tests yet"

clean:
	rm -f order_book order_book_debug order_book_asan

.PHONY: all debug asan test clean
