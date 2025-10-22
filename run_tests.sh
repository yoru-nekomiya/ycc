#!/bin/sh

YCC=./build/ycc
TEST_DIR=./tests
LIB="$TEST_DIR"/testlib.c
PASS=0
FAIL=0

FAILED_TESTS=""

mkdir -p "$TEST_DIR"/out

for file in "$TEST_DIR"/*.c; do
    #skip testlib.c
    [ "$(basename "$file")" = "testlib.c" ] && continue
    
    base=$(basename "$file" .c)
    filename=$(basename "$file")
    asm="$TEST_DIR/out/${base}.s"
    exe="$TEST_DIR/out/${base}.out"

    #echo "=== Testing $file ==="

    #compile by ycc
    $YCC "$file" > "$asm"
    if [ $? -ne 0 ]; then
	echo "Compilation failed for $file"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $filename (compile error)" || FAILED_TESTS="$FAILED_TESTS\n - $filename (compile error)"
	continue
    fi

    #assemble and link
    gcc -z noexecstack -o "$exe" "$asm" "$LIB"
    if [ $? -ne 0 ]; then
	echo "Linking failed for $file"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $filename (asm error)" || FAILED_TESTS="$FAILED_TESTS\n - $filename (asm error)"
	continue
    fi

    #run and test
    "$exe"
    result=$?

    if [ "$result" -eq 0 ]; then
	echo "Passed: $file"
	PASS=$((PASS+1))
    else
	echo "Failed: $file"
	FAIL=$((FAIL+1))
	[ -z "$FAILED_TESTS" ] && FAILED_TESTS=" - $filename (runtime error)" || FAILED_TESTS="$FAILED_TESTS\n - $filename (runtime error)"
    fi
done

echo "======================"
echo "PASS: $PASS"
echo "FAIL: $FAIL"
if [ -z "$FAILED_TESTS" ]; then
    echo "All tests are passed!"
else
    echo "Some tests are failed:"
    printf "$FAILED_TESTS\n"
    exit 1
fi
