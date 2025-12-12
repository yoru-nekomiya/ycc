#!/bin/sh

set -e

YCC=./build/ycc
GCC=gcc
CLANG=clang
BENCH_DIR=./bench
#OUT_DIR=./bench/out
GLOBAL_REPORT="$BENCH_DIR/report.txt"
TTY_DEVICE=/dev/tty

echo "Benchmark Report" > "$GLOBAL_REPORT"
echo "=================" >> "$GLOBAL_REPORT"
echo "" >> "$GLOBAL_REPORT"

# target benchmark
TARGETS="$@"
if [ -z "$TARGETS" ]; then
    # if no targets specified, all benchmarks will be target.
    TARGETS=$(find "$BENCH_DIR" -maxdepth 1 -mindepth 1 -type d -printf "%f\n")
else
    for t in $TARGETS; do
        if [ ! -d "$BENCH_DIR/$t" ]; then
            echo "Error: benchmark directory '$t' does not exist in $BENCH_DIR"
            exit 1
        fi
    done
fi

echo "=== Compiling benchmarks ==="
for t in $TARGETS; do
    BENCH_PATH="$BENCH_DIR/$t"
    src="$BENCH_PATH/$t.c"
    OUT_DIR="$BENCH_PATH/out"

    if [ ! -d "$BENCH_PATH" ]; then
        echo "Error: benchmark directory '$t' does not exist in $BENCH_DIR"
        exit 1
    fi
    
    if [ ! -f "$src" ]; then
        echo "Error: $src not found"
        exit 1
    fi

    mkdir -p "$OUT_DIR"
    
    asm="$OUT_DIR/${t}.s"
    exe_ycc="$OUT_DIR/${t}_ycc.out"
    exe_gcc="$OUT_DIR/${t}_gcc.out"
    exe_clang="$OUT_DIR/${t}_clang.out"
    
    echo "Compiling benchmark: $t"
    $YCC "$src" > "$asm"
    $GCC -z noexecstack -fno-pie -no-pie -g -o "$exe_ycc" "$asm"
    #compile by gcc and clang
    $GCC -O2 "$src" -o "$exe_gcc"
    $CLANG -O2 "$src" -o "$exe_clang"
done

echo ""
echo "=== Running benchmarks ==="

for t in $TARGETS; do
    OUT_DIR="$BENCH_DIR/$t/out"
    
    exe_ycc="$OUT_DIR/${t}_ycc.out"
    exe_gcc="$OUT_DIR/${t}_gcc.out"
    exe_clang="$OUT_DIR/${t}_clang.out"

    LOCAL_REPORT="$OUT_DIR/report.txt"
    
    echo "Running benchmark: $t"
    echo "Benchmark: $t" > "$LOCAL_REPORT"
    echo "=================" >> "$LOCAL_REPORT"
    echo "" >> "$LOCAL_REPORT"

    echo -n "YCC: "
    ycc_time=$( { /usr/bin/time -p "$exe_ycc" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )

    echo -n "GCC(-O2): "
    gcc_time=$( { /usr/bin/time -p "$exe_gcc" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )

    echo -n "Clang(-O2): "
    clang_time=$( { /usr/bin/time -p "$exe_clang" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )
    
    ratio_gcc=$(awk "BEGIN { printf \"%.2f\", $gcc_time / $ycc_time }")
    ratio_clang=$(awk "BEGIN { printf \"%.2f\", $clang_time / $ycc_time }")

    echo "  YCC        : $ycc_time sec"
    echo "  GCC(-O2)   : $gcc_time sec"
    echo "  Clang(-O2) : $clang_time sec"
    echo "  Ratio (GCC(-O2)/YCC)   : $ratio_gcc x"
    echo "  Ratio (Clang(-O2)/YCC) : $ratio_clang x"
    echo ""

    {
	echo "  YCC        : $ycc_time sec"
	echo "  GCC(-O2)   : $gcc_time sec"
	echo "  Clang(-O2) : $clang_time sec"
	echo "  Ratio (GCC(-O2)/YCC)   : $ratio_gcc x"
	echo "  Ratio (Clang(-O2)/YCC) : $ratio_clang x"
	echo ""
    } >> "$LOCAL_REPORT"
    
    {
        echo "Benchmark: $t"
        echo "  YCC        : $ycc_time sec"
        echo "  GCC(-O2)   : $gcc_time sec"
        echo "  Clang(-O2) : $clang_time sec"
        echo "  Ratio (GCC(-O2)/YCC)   : $ratio_gcc x"
        echo "  Ratio (Clang(-O2)/YCC) : $ratio_clang x"
        echo ""
    } >> "$GLOBAL_REPORT"
done

echo ""
echo "All specified benchmarks finished."
echo "Global report generated at: $GLOBAL_REPORT"
