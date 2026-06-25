#!/bin/sh

set -e

SLCC=./build/slcc
SLCC_OPT="-opt -emit-lir -emit-cfg"
GCC=gcc
CLANG=clang
BENCH_DIR=./bench
GLOBAL_REPORT="$BENCH_DIR/report.txt"
SLCC_OUT="/tmp/slcc_out.txt"
SLCC_OPT_OUT="/tmp/slcc_opt_out.txt"
GCC_OUT="/tmp/gcc_out.txt"
CLANG_OUT="/tmp/clang_out.txt"
#TTY_DEVICE=/dev/tty
CHECKSUM_FAILURES=""

#ANSI color
GREEN="\033[1;32m"
RED="\033[1;31m"
RESET="\033[0m"

echo "Benchmark Report" > "$GLOBAL_REPORT"
echo "=================" >> "$GLOBAL_REPORT"
echo "" >> "$GLOBAL_REPORT"

# target benchmark
TARGETS="$@"
if [ -z "$TARGETS" ]; then
    # if no targets are specified, all benchmarks will be target.
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
    exe_slcc="$OUT_DIR/${t}_slcc.out"
    asm_opt="$OUT_DIR/${t}_opt.s"
    exe_slcc_opt="$OUT_DIR/${t}_slcc_opt.out"
    
    exe_gcc="$OUT_DIR/${t}_gcc.out"
    exe_clang="$OUT_DIR/${t}_clang.out"
    
    echo "Compiling benchmark: $t"
    $SLCC "$src" > "$asm"
    $GCC -z noexecstack -fno-pie -no-pie -o "$exe_slcc" "$asm"
    $SLCC $SLCC_OPT "$src" > "$asm_opt"
    $GCC -z noexecstack -fno-pie -no-pie -o "$exe_slcc_opt" "$asm_opt"
    
    #compile by gcc and clang
    $GCC -O2 "$src" -o "$exe_gcc"
    $CLANG -O2 "$src" -o "$exe_clang"

    # dot -> svg
    for dot_file in "$BENCH_PATH"/*.dot; do
        if [ -f "$dot_file" ]; then
            svg_file="${dot_file%.dot}.svg"
            dot -Tsvg "$dot_file" -o "$svg_file"
            mv "$svg_file" "$OUT_DIR/"
            # mv "$dot_file" "$OUT_DIR/"
        fi
    done
done

echo ""
echo "=== Running benchmarks ==="

for t in $TARGETS; do
    OUT_DIR="$BENCH_DIR/$t/out"
    
    exe_slcc="$OUT_DIR/${t}_slcc.out"
    exe_slcc_opt="$OUT_DIR/${t}_slcc_opt.out"
    exe_gcc="$OUT_DIR/${t}_gcc.out"
    exe_clang="$OUT_DIR/${t}_clang.out"

    LOCAL_REPORT="$OUT_DIR/report.txt"
    
    echo "Running benchmark: $t"
    echo "Benchmark: $t" > "$LOCAL_REPORT"
    echo "=================" >> "$LOCAL_REPORT"
    echo "" >> "$LOCAL_REPORT"

    #echo -n "SLCC: "
    #slcc_time=$( { /usr/bin/time -p "$exe_slcc" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )
    slcc_time=$( { /usr/bin/time -p "$exe_slcc" 1> "$SLCC_OUT"; } 2>&1 | awk '/real/ {print $2}' )
    SLCC_CHECKSUM=$(grep "Checksum" "$SLCC_OUT" | awk '{print $NF}' | tr -d '\n')

    slcc_opt_time=$( { /usr/bin/time -p "$exe_slcc_opt" 1> "$SLCC_OPT_OUT"; } 2>&1 | awk '/real/ {print $2}' )
    SLCC_OPT_CHECKSUM=$(grep "Checksum" "$SLCC_OPT_OUT" | awk '{print $NF}' | tr -d '\n')

    #echo -n "GCC(-O2): "
    #gcc_time=$( { /usr/bin/time -p "$exe_gcc" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )
    gcc_time=$( { /usr/bin/time -p "$exe_gcc" 1> "$GCC_OUT"; } 2>&1 | awk '/real/ {print $2}' )
    GCC_CHECKSUM=$(grep "Checksum" "$GCC_OUT" | awk '{print $NF}' | tr -d '\n')

    #echo -n "Clang(-O2): "
    #clang_time=$( { /usr/bin/time -p "$exe_clang" 1> $TTY_DEVICE; } 2>&1 | awk '/real/ {print $2}' )
    clang_time=$( { /usr/bin/time -p "$exe_clang" 1> "$CLANG_OUT"; } 2>&1 | awk '/real/ {print $2}' )
    CLANG_CHECKSUM=$(grep "Checksum" "$CLANG_OUT" | awk '{print $NF}' | tr -d '\n')

    # compare checksum-----
    COLOR_CODE="$GREEN"
    if [ "$GCC_CHECKSUM" != "$CLANG_CHECKSUM" ]; then
        CHECKSUM_STATUS="FAIL (Reference Checksum Mismatch: GCC=$GCC_CHECKSUM, Clang=$CLANG_CHECKSUM)"
	CHECKSUM_FAILURES="$CHECKSUM_FAILURES $t (Ref)"
	COLOR_CODE="$RED"
    elif [ "$SLCC_CHECKSUM" = "$GCC_CHECKSUM" ]; then
	if [ "$SLCC_OPT_CHECKSUM" = "$SLCC_CHECKSUM" ]; then	    
            CHECKSUM_STATUS="PASS (Checksum: $SLCC_CHECKSUM)"
	    COLOR_CODE="$GREEN"
	else
	    CHECKSUM_STATUS="FAIL (SLCC=$SLCC_CHECKSUM, SLCC_OPT=$SLCC_OPT_CHECKSUM)"
	    CHECKSUM_FAILURES="$CHECKSUM_FAILURES $t"
	    COLOR_CODE="$RED"
	fi
    else
        CHECKSUM_STATUS="FAIL (SLCC=$SLCC_CHECKSUM, Reference=$GCC_CHECKSUM)"
	CHECKSUM_FAILURES="$CHECKSUM_FAILURES $t"
	COLOR_CODE="$RED"
    fi    
    # ---------------------
    
    # output report-----
    ratio_gcc=$(awk "BEGIN { printf \"%.2f\", $gcc_time / $slcc_time }")
    ratio_clang=$(awk "BEGIN { printf \"%.2f\", $clang_time / $slcc_time }")
    ratio_gcc_opt=$(awk "BEGIN { printf \"%.2f\", $gcc_time / $slcc_opt_time }")
    ratio_clang_opt=$(awk "BEGIN { printf \"%.2f\", $clang_time / $slcc_opt_time }")

    echo "  SLCC Checksum       : $SLCC_CHECKSUM"
    echo "  SLCC_OPT Checksum   : $SLCC_OPT_CHECKSUM"
    echo "  GCC Checksum       : $GCC_CHECKSUM"
    echo "  Clang Checksum     : $CLANG_CHECKSUM"
    echo "  Checksum Check : ${COLOR_CODE}${CHECKSUM_STATUS}${RESET}"
    echo "  SLCC            : $slcc_time sec"
    echo "  SLCC_OPT        : $slcc_opt_time sec"
    echo "  GCC(-O2)       : $gcc_time sec"
    echo "  Clang(-O2)     : $clang_time sec"
    echo "  Ratio (GCC(-O2)/SLCC)       : $ratio_gcc x"
    echo "  Ratio (Clang(-O2)/SLCC)     : $ratio_clang x"
    echo "  Ratio (GCC(-O2)/SLCC_OPT)   : $ratio_gcc_opt x"
    echo "  Ratio (Clang(-O2)/SLCC_OPT) : $ratio_clang_opt x"
    echo ""

    {
	echo "  SLCC Checksum       : $SLCC_CHECKSUM"
	echo "  SLCC_OPT Checksum   : $SLCC_CHECKSUM"
	echo "  GCC Checksum       : $GCC_CHECKSUM"
	echo "  Clang Checksum     : $CLANG_CHECKSUM"
	echo "  Checksum Check : $CHECKSUM_STATUS"
	echo "  SLCC            : $slcc_time sec"
	echo "  SLCC_OPT        : $slcc_opt_time sec"
	echo "  GCC(-O2)       : $gcc_time sec"
	echo "  Clang(-O2)     : $clang_time sec"
	echo "  Ratio (GCC(-O2)/SLCC)       : $ratio_gcc x"
	echo "  Ratio (Clang(-O2)/SLCC)     : $ratio_clang x"
	echo "  Ratio (GCC(-O2)/SLCC_OPT)   : $ratio_gcc_opt x"
	echo "  Ratio (Clang(-O2)/SLCC_OPT) : $ratio_clang_opt x"
	echo ""
    } >> "$LOCAL_REPORT"
    
    {
        echo "Benchmark: $t"
	echo "  Checksum Check : $CHECKSUM_STATUS"
        echo "  SLCC            : $slcc_time sec"
	echo "  SLCC_OPT        : $slcc_opt_time sec"
        echo "  GCC(-O2)       : $gcc_time sec"
        echo "  Clang(-O2)     : $clang_time sec"
        echo "  Ratio (GCC(-O2)/SLCC)   : $ratio_gcc x"
        echo "  Ratio (Clang(-O2)/SLCC) : $ratio_clang x"
	echo "  Ratio (GCC(-O2)/SLCC_OPT)   : $ratio_gcc_opt x"
	echo "  Ratio (Clang(-O2)/SLCC_OPT) : $ratio_clang_opt x"
        echo ""
    } >> "$GLOBAL_REPORT"
done

rm -f "$SLCC_OUT" "$SLCC_OPT_OUT" "$GCC_OUT" "$CLANG_OUT"

echo ""
echo "========================================="
echo "--- FINAL CHECKUSM VALIDATION SUMMARY ---" >> "$GLOBAL_REPORT"

if [ -z "$CHECKSUM_FAILURES" ]; then
    SUCCESS_MSG="SUCCESS: All benchmarks passed the checksum validation."
    echo "${GREEN}${SUCCESS_MSG}${RESET}"
    echo "$SUCCESS_MSG" >> "$GLOBAL_REPORT"
else
    FAIL_MSG="FAILURE: The following benchmarks had CHECKUM MISMATCHES:"
    echo "${RED}${FAIL_MSG}${RESET}"
    echo "$FAIL_MSG" >> "$GLOBAL_REPORT"
    
    echo "" >> "$GLOBAL_REPORT"
    echo "--- Failed Benchmarks ---"
    
    echo "$CHECKSUM_FAILURES" | tr ' ' '\n' | grep -v '^$' | while read bench_name; do
        echo "- $bench_name"
        echo "- $bench_name" >> "$GLOBAL_REPORT"
    done
    echo "-------------------------" >> "$GLOBAL_REPORT"
fi

echo "========================================="
echo "All specified benchmarks finished."
echo "Global report generated at: $GLOBAL_REPORT"
