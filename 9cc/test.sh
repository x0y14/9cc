#!/bin/bash
assert() {
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 0
assert 42 42
assert 5 "1+2+2"
assert 5 "10-7+2"
assert 3 " 1 + 3 + 5 - 4 -2"

echo OK
