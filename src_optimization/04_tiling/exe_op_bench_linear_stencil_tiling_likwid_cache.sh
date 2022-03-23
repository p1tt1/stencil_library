#!/usr/bin/env bash

NODE_ID=${NODE_ID:-undefined_node}
SCRIPT_PATH="$(readlink -f "$0")"

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

log() {
  echo -e "$BLUE > $1 $NORMAL"
}
warning() {
  echo -e "$YELLOW > WARNING - $1 $NORMAL"
}

error() {
  echo ""
  echo -e "$RED >>> ERROR - $1 $NORMAL" >&2
}

output() {
  echo $1 >> $OUTPUT_FILE
}

log_telegram() {
  curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}"
}

log_telegram_silent() {
  curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}&disable_notification=true"
}

timestamp() {
  date +%s
}

elapsedTimeMessage() {
  local l_elapsed=$(( $2 - $1 ))
  eval "echo Elapsed time: $(date -ud "@$l_elapsed" +'$((%s/3600/24)) days %H hr %M min %S sec')"
}

if [ -z ${TELEGRAM_BOT_TOKEN} ]; then
  error "TELEGRAM_BOT_TOKEN isn't defined."
  exit 1
fi
if [ -z ${TELEGRAM_CHAT_ID} ]; then
  error "TELEGRAM_CHAT_ID isn't defined."
  exit 1
fi

APP="./exe_op_bench_linear_stencil_tiling_likwid"
OUTPUT_FILE="${APP}_cache_${NODE_ID}.csv"

BENCH_CMD="likwid-perfctr -C S0:1 -f -g CACHE -m -O ${APP}"

# TILINGS_COLS=(4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536)
# TILINGS_LEVELS_ROWS=(1 2 4 8 16 32 64 128 256)

# TILINGS_COLS=(4 8 16 32 64 128 256 512 1024)
# TILINGS_LEVELS_ROWS=(1 2 4 8 16 32 64 128 256 512 1024)

TILINGS_COLS=(2048)
TILINGS_LEVELS_ROWS=(1 2 4 8 16 32 64 128 256 512 1024)

# OBJ_COLS=${TILINGS_COLS[${#TILINGS_COLS[@]}-1]}
# OBJ_LEVELS_ROWS=${TILINGS_LEVELS_ROWS[${#TILINGS_LEVELS_ROWS[@]}-1]}
OBJ_COLS=2048
OBJ_LEVELS_ROWS=2048

OBJ_COLS_RESULT=$(expr $OBJ_COLS + 2)
OBJ_LEVELS_ROWS_RESULT=$(expr $OBJ_LEVELS_ROWS + 2)

TIME_BENCHMARK_STARTED=$(timestamp)
log "benchmark started ($SCRIPT_PATH)"
log_telegram_silent "benchmark started ($SCRIPT_PATH)"

#
# clear file
#
> $OUTPUT_FILE
output "node_id,impl_id,time_id,obj_levels_rows,obj_cols,n_tiling_levels_rows,n_tiling_cols,actual_cpu_clock,max_cpu_clock,retired_instructions,cpu_clocks_unhalted,data_cache_accesses,data_cache_refills_all,runtime,runtime_unhalted,clock,cpi,data_cache_requests,data_cache_request_rate,data_cache_misses,data_cache_miss_rate,data_cache_miss_ratio"

for TILING_LEVELS_ROW in ${TILINGS_LEVELS_ROWS[@]}; do
  for TILING_COLS in ${TILINGS_COLS[@]}; do
    CURRENT_CMD="$BENCH_CMD $OBJ_COLS $OBJ_LEVELS_ROWS $OBJ_LEVELS_ROWS $TILING_COLS $TILING_LEVELS_ROW $TILING_LEVELS_ROW"
    log "\$ $CURRENT_CMD"

    TIME_CURRENT=$(timestamp)
    log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
    log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
    log "\$ $CURRENT_CMD"
    log_telegram_silent "> \$ $CURRENT_CMD"

    CURRENT_OUTPUT=$($CURRENT_CMD)
    #
    # NOTE: test cmd
    #
    # CURRENT_OUTPUT=$($BENCH_CMD 8 8 8 4 4 4)

    ACTUAL_CPU_CLOCK=$(echo "$CURRENT_OUTPUT"         | grep "ACTUAL_CPU_CLOCK"         | cut -d ',' -f 3)
    MAX_CPU_CLOCK=$(echo "$CURRENT_OUTPUT"            | grep "MAX_CPU_CLOCK"            | cut -d ',' -f 3)
    RETIRED_INSTRUCTIONS=$(echo "$CURRENT_OUTPUT"     | grep "RETIRED_INSTRUCTIONS"     | cut -d ',' -f 3)
    CPU_CLOCKS_UNHALTED=$(echo "$CURRENT_OUTPUT"      | grep "CPU_CLOCKS_UNHALTED"      | cut -d ',' -f 3)
    DATA_CACHE_ACCESSES=$(echo "$CURRENT_OUTPUT"      | grep "DATA_CACHE_ACCESSES"      | cut -d ',' -f 3)
    DATA_CACHE_REFILLS_ALL=$(echo "$CURRENT_OUTPUT"   | grep "DATA_CACHE_REFILLS_ALL"   | cut -d ',' -f 3)
    RUNTIME=$(echo "$CURRENT_OUTPUT"                  | grep "Runtime (RDTSC) \[s\]"    | cut -d ',' -f 2)
    RUNTIME_UNHALTED=$(echo "$CURRENT_OUTPUT"         | grep "Runtime unhalted \[s\]"   | cut -d ',' -f 2)
    CLOCK=$(echo "$CURRENT_OUTPUT"                    | grep "Clock \[MHz\]"            | cut -d ',' -f 2)
    CPI=$(echo "$CURRENT_OUTPUT"                      | grep "CPI"                      | cut -d ',' -f 2)
    DATA_CACHE_REQUESTS=$(echo "$CURRENT_OUTPUT"      | grep "data cache requests"      | cut -d ',' -f 2)
    DATA_CACHE_REQUEST_RATE=$(echo "$CURRENT_OUTPUT"  | grep "data cache request rate"  | cut -d ',' -f 2)
    DATA_CACHE_MISSES=$(echo "$CURRENT_OUTPUT"        | grep "data cache misses"        | cut -d ',' -f 2)
    DATA_CACHE_MISS_RATE=$(echo "$CURRENT_OUTPUT"     | grep "data cache miss rate"     | cut -d ',' -f 2)
    DATA_CACHE_MISS_RATIO=$(echo "$CURRENT_OUTPUT"    | grep "data cache miss ratio"    | cut -d ',' -f 2)

    # echo ACTUAL_CPU_CLOCK: $ACTUAL_CPU_CLOCK
    # echo MAX_CPU_CLOCK: $MAX_CPU_CLOCK
    # echo RETIRED_INSTRUCTIONS: $RETIRED_INSTRUCTIONS
    # echo CPU_CLOCKS_UNHALTED: $CPU_CLOCKS_UNHALTED
    # echo DATA_CACHE_ACCESSES: $DATA_CACHE_ACCESSES
    # echo DATA_CACHE_REFILLS_ALL: $DATA_CACHE_REFILLS_ALL
    # echo RUNTIME: $RUNTIME
    # echo RUNTIME_UNHALTED: $RUNTIME_UNHALTED
    # echo CLOCK: $CLOCK
    # echo CPI: $CPI
    # echo DATA_CACHE_REQUESTS: $DATA_CACHE_REQUESTS
    # echo DATA_CACHE_REQUEST_RATE: $DATA_CACHE_REQUEST_RATE
    # echo DATA_CACHE_MISSES: $DATA_CACHE_MISSES
    # echo DATA_CACHE_MISS_RATE: $DATA_CACHE_MISS_RATE
    # echo DATA_CACHE_MISS_RATIO: $DATA_CACHE_MISS_RATIO

    output "$NODE_ID,linear_stencil_tiling,time_apply,$OBJ_LEVELS_ROWS_RESULT,$OBJ_COLS_RESULT,$TILING_LEVELS_ROW,$TILING_COLS,$ACTUAL_CPU_CLOCK,$MAX_CPU_CLOCK,$RETIRED_INSTRUCTIONS,$CPU_CLOCKS_UNHALTED,$DATA_CACHE_ACCESSES,$DATA_CACHE_REFILLS_ALL,$RUNTIME,$RUNTIME_UNHALTED,$CLOCK,$CPI,$DATA_CACHE_REQUESTS,$DATA_CACHE_REQUEST_RATE,$DATA_CACHE_MISSES,$DATA_CACHE_MISS_RATE,$DATA_CACHE_MISS_RATIO"
  done
  echo ---
done

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "benchmark finished"
log "=================="
log_telegram "benchmark finished"
log_telegram "=================="
