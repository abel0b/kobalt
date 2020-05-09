CC ?= gcc
CFLAGS += -std=c99 -Wall -Wextra -Iinclude
LDLIBS = -lm

.PHONY: all release debug native %test clean%

all: release

release: target/release/kbc
debug: target/debug/kbc
native: target/native/kbc

target/release/%: CFLAGS += -O3
target/debug/% target/test/%: CFLAGS += -DDEBUG -ggdb3 -O0
target/native/%: CFLAGS += -O3 -march=native

target/%/kbc: src/main.c src/options.c src/source.c src/lexer.c src/token.c src/parser.c include/kobalt/options.h include/kobalt/source.h include/kobalt/lexer.h include/kobalt/lexer.h include/kobalt/parser.h | target/%
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

test: unit-test e2e-test

e2e-test: target/test/kbc
	bash test/e2e/run.sh

unit-test: target/test/unit-test
	./target/test/unit-test

target/test/unit-test: test/unit/main.c | target/test
	$(CC) $(CFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -rf target

target/release target/debug target/test target/native:
	mkdir -p $@

compile_flags.txt: Makefile
	printf '%s\n' $(CFLAGS) > $@
