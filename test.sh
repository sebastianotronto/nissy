#!/bin/sh

# Warning: tests should be in the correct order!
# Every testing module assumes that modules it depends on pass their tests.
all_modules=fst

test_module() {
	modules=$@
	for m in modules; do
		./tests/nissy_test $m
	done
}

if [ -n "$@" ]; then
	test_module $@
else
	test_module $all_modules
fi
rm tests/nissy_test
