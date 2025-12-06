#!/bin/sh

set -e

YCC=./build/ycc
GCC=gcc
CLANG=clang
BENCH_DIR=./bench
OUT_DIR=./bench/out
REPORT="$OUT_DIR/report.txt"
TTY_DEVICE=/dev/tty

mkdir -p "$OUT_DIR"
echo "Benchmark Report" > "$REPORT"
echo "=================" >> "$REPORT"
echo "" >> "$REPORT"

echo "=== Compiling all benchmarks ==="
for src in "$BENCH_DIR"/*.c; do
    name=$(basename "$src" .c)
    asm="$OUT_DIR/${name}.s"
    exe="$OUT_DIR/${name}.out"
    exe_gcc="$OUT_DIR/${name}_gcc.out"
    exe_clang="$OUT_DIR/${name}_clang.out"
    
    echo "Compiling benchmark: $name"
    $YCC "$src" > "$asm"
    $GCC -z noexecstack -fno-pie -no-pie -g -o "$exe" "$asm"
    #compile by gcc and clang
    $GCC -O2 "$src" -o "$exe_gcc"
    $CLANG -O2 "$src" -o "$exe_clang"
done

echo ""
echo "=== Running all benchmarks ==="

for src in "$BENCH_DIR"/*.c; do
    name=$(basename "$src" .c)
    exe="$OUT_DIR/${name}.out"
    exe_gcc="$OUT_DIR/${name}_gcc.out"
    exe_clang="$OUT_DIR/${name}_clang.out"

    echo "Running benchmark: $name"

    echo -n "YCC: "
    ycc_time=$( { /usr/bin/time -p "$exe" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )

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
        echo "Benchmark: $name"
        echo "  YCC        : $ycc_time sec"
        echo "  GCC(-O2)   : $gcc_time sec"
        echo "  Clang(-O2) : $clang_time sec"
        echo "  Ratio (GCC(-O2)/YCC)   : $ratio_gcc x"
        echo "  Ratio (Clang(-O2)/YCC) : $ratio_clang x"
        echo ""
    } >> "$REPORT"
done

echo ""
echo "All benchmarks finished."
echo "Report generated at: $REPORT"
