#!/bin/sh

YCC=./build/ycc
TEST_DIR=./tests
LIB="$TEST_DIR"/testlib.c
PASS=0
FAIL=0

FAILED_TESTS=""

mkdir -p "$TEST_DIR"/out

#ANSI color
GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
RESET="\033[0m"

for file in $(find "$TEST_DIR" -name "*.c"); do
    #skip testlib.c
    [ "$(basename "$file")" = "testlib.c" ] && continue

    #create output directory
    rel_dir=$(dirname "${file#$TEST_DIR/}")
    rel_path=${file#"$TEST_DIR"/}
    mkdir -p "$TEST_DIR/out/$rel_dir"
    
    base=$(basename "$file" .c)
    filename=$(basename "$file")
    asm="$TEST_DIR/out/$rel_dir/${base}.s"
    exe="$TEST_DIR/out/$rel_dir/${base}.out"

    #echo "=== Testing $file ==="

    #compile by ycc
    $YCC "$file" > "$asm"
    if [ $? -ne 0 ]; then
	printf "[${RED}FAIL${RESET}] %s (compile error)\n" "$rel_path"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $rel_path (compile error)" || FAILED_TESTS="$FAILED_TESTS\n - $rel_path (compile error)"
	continue
    fi

    #assemble and link
    gcc -z noexecstack -fno-pie -no-pie -o "$exe" "$asm" "$LIB"
    if [ $? -ne 0 ]; then
	printf "[${RED}FAIL${RESET}] %s (asm or link error)\n" "$rel_path"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $rel_path (asm or link error)" || FAILED_TESTS="$FAILED_TESTS\n - $rel_path (asm or link error)"
	continue
    fi

    #run and test
    "$exe"
    result=$?

    if [ "$result" -eq 0 ]; then
	printf "[${GREEN}PASS${RESET}] %s\n" "$rel_path"
	PASS=$((PASS+1))
    else
	printf "[${RED}FAIL${RESET}] %s (runtime error)\n" "$rel_path"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $rel_path (runtime error)" || FAILED_TESTS="$FAILED_TESTS\n - $rel_path (runtime error)"
    fi
done

echo "======================"
printf "${GREEN}PASS${RESET}: %d\n" "$PASS"
printf "${RED}FAIL${RESET}: %d\n" "$FAIL"
if [ -z "$FAILED_TESTS" ]; then
    printf "${GREEN}All tests are passed!${RESET}\n"
else
    printf "${YELLOW}Some tests are failed:${RESET}\n"
    printf "$FAILED_TESTS\n"
    exit 1
fi
