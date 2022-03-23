#!/usr/bin/env bash

#
# CONFIG
#

OBJ_COLS=${1:-2048}
OBJ_ROWS=${2:-2048}
OBJ_LEVELS=${3:-2048}

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
  curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}" >/dev/null
}

log_telegram_silent() {
  curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}&disable_notification=true" >/dev/null
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
CMD_BENCH_PREFIX_MEM="likwid-perfctr -C S0:1 -f -g MEM -m -O ${APP}"
CMD_BENCH_PREFIX_FLOPS_DP="likwid-perfctr -C S0:1 -f -g FLOPS_DP -m -O ${APP}"

TILING_COLS=0
TILING_ROWS=0
TILING_LEVELS=0

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
output "node_id,bench_id,impl_id,region_id,obj_cols,obj_rows,obj_levels,tiling_cols,tiling_rows,tiling_levels,runtime,clock,cpi,memory_bandwidth,memory_data_volume,performance"

#
# ------------------------------
#

#
# COMMAND EXECUTION
#

CMD_MEM="$CMD_BENCH_PREFIX_MEM $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS"
# CMD_MEM="$CMD_BENCH_PREFIX_MEM 8 8 8"

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "\$ $CMD_MEM"
log_telegram_silent "> \$ $CMD_MEM"

OUTPUT_CMD_MEM=$($CMD_MEM)
# echo "$OUTPUT_CMD_MEM"

CMD_FLOPS_DP="$CMD_BENCH_PREFIX_FLOPS_DP $OBJ_COLS $OBJ_ROWS $OBJ_ROWS"
# CMD_FLOPS_DP="$CMD_BENCH_PREFIX_FLOPS_DP 8 8 8"

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "\$ $CMD_FLOPS_DP"
log_telegram_silent "> \$ $CMD_FLOPS_DP"

OUTPUT_CMD_FLOPS_DB=$($CMD_FLOPS_DP)
# echo "$OUTPUT_CMD_FLOPS_DB"

#
# ------------------------------
#

#
# PARSE RESULTS
#

RUNTIME=$(echo "$OUTPUT_CMD_MEM"            | grep "Runtime (RDTSC) \[s\]"         | cut -d ',' -f 2)
CLOCK=$(echo "$OUTPUT_CMD_MEM"              | grep "Clock \[MHz\]"                 | cut -d ',' -f 2)
CPI=$(echo "$OUTPUT_CMD_MEM"                | grep "CPI"                           | cut -d ',' -f 2)
MEMORY_BANDWIDTH=$(echo "$OUTPUT_CMD_MEM"   | grep "Memory bandwidth \[MBytes/s\]" | cut -d ',' -f 2)
MEMORY_DATA_VOLUME=$(echo "$OUTPUT_CMD_MEM" | grep "Memory data volume \[GBytes\]" | cut -d ',' -f 2)
PERFORMANCE=$(echo "$OUTPUT_CMD_FLOPS_DB"   | grep "DP \[MFLOP/s\]"                | cut -d ',' -f 2)

RUNTIME_FLOPS=$(echo "$OUTPUT_CMD_FLOPS_DB"            | grep "Runtime (RDTSC) \[s\]"         | cut -d ',' -f 2)
echo RUNTIME_MEM: $RUNTIME
echo RUNTIME_FLOPS: $RUNTIME_FLOPS

output "$NODE_ID,$BENCH_ID,$ALGORTIHM_ID,$REGION_ID,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$TILING_COLS,$TILING_ROWS,$TILING_LEVELS,$RUNTIME,$CLOCK,$CPI,$MEMORY_BANDWIDTH,$MEMORY_DATA_VOLUME,$PERFORMANCE"

TIME_CURRENT=$(timestamp)
log "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_telegram_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log "benchmark finished"
log "=================="
log_telegram "benchmark finished"
log_telegram "=================="
