#!/usr/bin/env bash

#
# CONFIG
#

# TILINGS_COLS=(4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536)
# TILINGS_LEVELS_ROWS=(1 2 4 8 16 32 64 128 256)
TILINGS_COLS=(4 8 16 32 64 128 256 512 1024)
TILINGS_LEVELS_ROWS=(1 2 4 8 16 32 64 128 256 512 1024)

OBJ_COLS=2048
OBJ_LEVELS_ROWS=2048

#
# ------------------------------
#

#
# functions
#

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
  curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}&disable_notificationclear=true"
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

#
# ------------------------------
#

#
# PROPERTIES
#

ALGORTIHM_ID="linear_stencil_notiling"
APP="./exe_op_bench_${ALGORTIHM_ID}_likwid"
CMD_BENCH_PREFIX="likwid-perfctr -C S0:1 -f -g CACHE -m -O ${APP}"

NODE_ID=${NODE_ID:-undefined_node}
BENCH_ID="roofline"
REGION_ID="time_apply"
SCRIPT_PATH="$(readlink -f "$0")"

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="${APP}_${BENCH_ID}_${NODE_ID}.csv"

OBJ_COLS_RESULT=$(expr $OBJ_COLS + 2)
OBJ_ROWS_RESULT=$(expr $OBJ_ROWS + 2)
OBJ_LEVELS_RESULT=$(expr $OBJ_LEVELS + 2)

TIME_BENCHMARK_STARTED=$(timestamp)
log "benchmark started ($SCRIPT_PATH)"
log_telegram_silent "benchmark started ($SCRIPT_PATH)"

#
# clear file
#
> $OUTPUT_FILE
output "node_id,bench_id,impl_id,region_id,obj_cols,obj_rows,obj_levels,tiling_cols,tiling_rows,tiling_levels,actual_cpu_clock,max_cpu_clock,retired_instructions,cpu_clocks_unhalted,data_cache_accesses,data_cache_refills_all,runtime,runtime_unhalted,clock,cpi,data_cache_requests,data_cache_request_rate,data_cache_misses,data_cache_miss_rate,data_cache_miss_ratio"

#
# ------------------------------
#

#
# COMMAND EXECUTION
#

CMD_MEM="$CMD_BENCH_PREFIX $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS"
# CMD_MEM="$CMD_BENCH_PREFIX 8 8 8"

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "\$ $CMD_MEM"
log_telegram_silent "> \$ $CMD_MEM"

OUTPUT_CMD=$($CMD_MEM)

#
# ------------------------------
#

#
# PARSE RESULTS
#
ACTUAL_CPU_CLOCK=$(echo "$OUTPUT_CMD"         | grep "ACTUAL_CPU_CLOCK"         | cut -d ',' -f 3)
MAX_CPU_CLOCK=$(echo "$OUTPUT_CMD"            | grep "MAX_CPU_CLOCK"            | cut -d ',' -f 3)
RETIRED_INSTRUCTIONS=$(echo "$OUTPUT_CMD"     | grep "RETIRED_INSTRUCTIONS"     | cut -d ',' -f 3)
CPU_CLOCKS_UNHALTED=$(echo "$OUTPUT_CMD"      | grep "CPU_CLOCKS_UNHALTED"      | cut -d ',' -f 3)
DATA_CACHE_ACCESSES=$(echo "$OUTPUT_CMD"      | grep "DATA_CACHE_ACCESSES"      | cut -d ',' -f 3)
DATA_CACHE_REFILLS_ALL=$(echo "$OUTPUT_CMD"   | grep "DATA_CACHE_REFILLS_ALL"   | cut -d ',' -f 3)
RUNTIME=$(echo "$OUTPUT_CMD"                  | grep "Runtime (RDTSC) \[s\]"    | cut -d ',' -f 2)
RUNTIME_UNHALTED=$(echo "$OUTPUT_CMD"         | grep "Runtime unhalted \[s\]"   | cut -d ',' -f 2)
CLOCK=$(echo "$OUTPUT_CMD"                    | grep "Clock \[MHz\]"            | cut -d ',' -f 2)
CPI=$(echo "$OUTPUT_CMD"                      | grep "CPI"                      | cut -d ',' -f 2)
DATA_CACHE_REQUESTS=$(echo "$OUTPUT_CMD"      | grep "data cache requests"      | cut -d ',' -f 2)
DATA_CACHE_REQUEST_RATE=$(echo "$OUTPUT_CMD"  | grep "data cache request rate"  | cut -d ',' -f 2)
DATA_CACHE_MISSES=$(echo "$OUTPUT_CMD"        | grep "data cache misses"        | cut -d ',' -f 2)
DATA_CACHE_MISS_RATE=$(echo "$OUTPUT_CMD"     | grep "data cache miss rate"     | cut -d ',' -f 2)
DATA_CACHE_MISS_RATIO=$(echo "$OUTPUT_CMD"    | grep "data cache miss ratio"    | cut -d ',' -f 2)

for TILING_LEVELS_ROW in ${TILINGS_LEVELS_ROWS[@]}; do
  for TILING_COLS in ${TILINGS_COLS[@]}; do
    output "$NODE_ID,$BENCH_ID,$ALGORTIHM_ID,$REGION_ID,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$TILING_COLS,$TILING_LEVELS_ROW,$TILING_LEVELS_ROW,$ACTUAL_CPU_CLOCK,$MAX_CPU_CLOCK,$RETIRED_INSTRUCTIONS,$CPU_CLOCKS_UNHALTED,$DATA_CACHE_ACCESSES,$DATA_CACHE_REFILLS_ALL,$RUNTIME,$RUNTIME_UNHALTED,$CLOCK,$CPI,$DATA_CACHE_REQUESTS,$DATA_CACHE_REQUEST_RATE,$DATA_CACHE_MISSES,$DATA_CACHE_MISS_RATE,$DATA_CACHE_MISS_RATIO"
  done
done

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "benchmark finished"
log "=================="
log_telegram "benchmark finished"
log_telegram "=================="
