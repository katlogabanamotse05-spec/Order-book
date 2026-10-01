CC    = cc
WARN  = -std=c11 -Wall -Wextra -Wpedantic -Werror
SRC   = $(wildcard src/*.c)
ASAN  = -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined -DCHECK_INVARIANTS

all: order_book

order_book: $(SRC:src/%.c=build/rel/%.o)
	$(CC) $(WARN) -O2 $^ -o $@

build/rel/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(WARN) -O2 -Iinclude -MMD -MP -c $< -o $@

asan: order_book_asan
order_book_asan: $(SRC:src/%.c=build/asan/%.o)
	$(CC) $(WARN) $(ASAN) $^ -o $@

build/asan/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(WARN) $(ASAN) -Iinclude -MMD -MP -c $< -o $@

test: asan
	@echo "no tests yet"

clean:
	rm -rf build order_book order_book_asan

-include $(wildcard build/*/*.d)
.PHONY: all asan test clean
