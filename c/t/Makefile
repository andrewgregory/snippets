CFLAGS += -Wall -Wextra -g -I..

PROVE ?= prove

TESTS = \
		10-asprintf.t \
		10-fuzzy.t \
		10-mbscasecmp.t \
		10-pathcmp.t \
		10-strjoin.t \
		10-systemv.t \
		10-wordsplit.t

%.t: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

tests: $(TESTS)

check: tests
	$(PROVE) $(TESTS)

all: tests

gcov: CC = gcc
gcov: CFLAGS += -fprofile-arcs -ftest-coverage
gcov: check
	gcov $(TESTS)

clean:
	$(RM) $(TESTS)
	$(RM) *.gcda *.gcno *.gcov

.PHONY: all clean check tests
