#!/usr/bin/env bash

#
# CONFIG
#

THREADS_MIN=1
THREADS_MAX=128

OBJ_COLS=${1:-2048}
OBJ_ROWS=${2:-2048}
OBJ_LEVELS=${3:-2048}
TILING_COLS=${4:-1024}
TILING_ROWS=${5:-4}
TILING_LEVELS=${6:-4}

ENABLE_TELEGRAM=true

#
# ------------------------------
#

#
# FUNCTIONS
#

filename() {
  echo "$(basename $(readlink -f $1))"
}

log_console() {
  echo -e "$BLUE $1 $NORMAL"
}
warning_console() {
  echo -e "$YELLOW > WARNING - $1 $NORMAL"
}

error_console() {
  echo ""
  echo -e "$RED >>> ERROR - $1 $NORMAL" >&2
}

output() {
  echo $1 >> $OUTPUT_FILE
}

log_telegram() {
  if [ "${ENABLE_TELEGRAM}" = true ]; then
    curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}" >/dev/null
  fi
}

log_telegram_silent() {
  if [ "${ENABLE_TELEGRAM}" = true ]; then
    curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}&disable_notification=true" >/dev/null
  fi
}

log_everywhere() {
  log_console "$1"
  log_telegram "$1"
}

log_everywhere_silent() {
  log_console "$1"
  log_telegram_silent "$1"
}

timestamp() {
  date +%s
}

elapsedTimeMessage() {
  local l_elapsed=$(( $2 - $1 ))
  eval "echo Elapsed time: $(date -ud "@$l_elapsed" +'$((%s/3600/24)) days %H hr %M min %S sec')"
}

thread_config(){
  local thread_id_max=$(( $1 - 1 ))
  if [ $thread_id_max -eq 0 ]
  then
    local config="N:0"
  else
    local config="N:0-$thread_id_max"
  fi
  echo $config
}

bench_cmd_likwid_perfctr(){
  echo "likwid-perfctr -C $(thread_config $1) -f -g FLOPS_DP -O ${APP_PATH} $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $TILING_COLS $TILING_ROWS $TILING_LEVELS"
}

bench_cmd_likwid_pin(){
  echo "likwid-pin -C $(thread_config $1) ${APP_PATH} $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $TILING_COLS $TILING_ROWS $TILING_LEVELS"
}

#
# ------------------------------
#

#
# ENVIRONMENT CHECK
#

if [ -z ${TELEGRAM_BOT_TOKEN} ]; then
  error_console "TELEGRAM_BOT_TOKEN isn't defined."
  exit 1
fi
if [ -z ${TELEGRAM_CHAT_ID} ]; then
  error_console "TELEGRAM_CHAT_ID isn't defined."
  exit 1
fi

#
# ------------------------------
#

#
# PROPERTIES
#

APP_PATH=./e_linear_stencil_para_tiling
APP_ID=$(filename $APP_PATH)

NODE_ID=${NODE_ID:-undefined_node}
BENCH_ID="threads"
SCALING="strong"
SCRIPT_PATH=$(readlink -f $0)

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="${APP_ID}_efficiency_${SCALING}_${NODE_ID}.csv"

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_PATH)"

THREAD_LIST=()

#
# ------------------------------
#

#
# GENERATE BENCH LIST WEAK SCALING
#

THREADS=$THREADS_MIN
while [ $THREADS -le $THREADS_MAX ]
do
  THREAD_LIST+=($THREADS)
  THREADS=$(( $THREADS + 1 ))
done

#
# clear file
#
> $OUTPUT_FILE
output "node_id,scaling,impl_id,obj_cols,obj_rows,obj_levels,threads,runtime,clock,cpi,flops_dp,efficiency_apply,efficiency_routine"

#
# ------------------------------
#

#
# EXECUTION
#

for INDEX in ${!THREAD_LIST[@]}; do
  THREADS=${THREAD_LIST[$INDEX]}

  CURRENT_CMD=$(bench_cmd_likwid_pin $THREADS)

  log_everywhere_silent "($(( $INDEX + 1 ))/${#THREAD_LIST[@]}) \$ $CURRENT_CMD"

  CURRENT_OUTPUT=$($CURRENT_CMD)

  #
  # parse results
  #

  IMPL_ID=$(echo "$CURRENT_OUTPUT"    | grep "IMPL_ID_IMPL"                | cut -d ',' -f 2)
  OBJ_COLS_RESULT=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_COLS_IMPL"               | cut -d ',' -f 2)
  OBJ_ROWS_RESULT=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_ROWS_IMPL"               | cut -d ',' -f 2)
  OBJ_LEVELS_RESULT=$(echo "$CURRENT_OUTPUT" | grep "OBJ_LEVELS_IMPL"             | cut -d ',' -f 2)
  RUNTIME_APPLY=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_APPLY_IMPL"                | cut -d ',' -f 2)
  RUNTIME_ROUTINE=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_ROUTINE_IMPL"                | cut -d ',' -f 2)

  echo "$CURRENT_OUTPUT"

  if [ $THREADS -eq 1 ]
  then
    CLOCK=$(echo "$CURRENT_OUTPUT"      | grep "Clock \[MHz\]"          | cut -d ',' -f 2)
    CPI=$(echo "$CURRENT_OUTPUT"        | grep "CPI"                    | cut -d ',' -f 2)
    FLOPS_DP=$(echo "$CURRENT_OUTPUT"   | grep "DP \[MFLOP/s\]"         | cut -d ',' -f 2)
  else
    CLOCK=$(echo "$CURRENT_OUTPUT"      | grep "Clock \[MHz\] STAT"          | cut -d ',' -f 2)
    CPI=$(echo "$CURRENT_OUTPUT"        | grep "CPI STAT"                    | cut -d ',' -f 2)
    FLOPS_DP=$(echo "$CURRENT_OUTPUT"   | grep "DP \[MFLOP/s\] STAT"         | cut -d ',' -f 2)
  fi

  if [ $THREADS -eq $THREADS_MIN ]; then
    RUNTIME_REF_APPLY=$RUNTIME_APPLY
    RUNTIME_REF_ROUTINE=$RUNTIME_ROUTINE
  fi

  EFFICIENCY_APPLY=$(python3 -c "print($RUNTIME_REF_APPLY/($THREADS*$RUNTIME_APPLY))")
  EFFICIENCY_ROUTINE=$(python3 -c "print($RUNTIME_REF_ROUTINE/($THREADS*$RUNTIME_ROUTINE))")

  output "$NODE_ID,$SCALING,$IMPL_ID,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$THREADS,$RUNTIME,$CLOCK,$CPI,$FLOPS_DP,$EFFICIENCY_APPLY,$EFFICIENCY_ROUTINE"

  TIME_CURRENT=$(timestamp)
  log_everywhere_silent "=> Runtime: $RUNTIME_ROUTINE sec"
  log_everywhere_silent "=> Efficiency apply: $EFFICIENCY_APPLY"
  log_everywhere_silent "=> Efficiency routine: $EFFICIENCY_ROUTINE"
  log_everywhere_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
  log_everywhere_silent "------------------"

done

#
# ------------------------------
#

#
# COMPLETED
#

TIME_CURRENT=$(timestamp)
log_everywhere_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
log_everywhere "benchmark finished"
log_everywhere_silent "=================="

exit 0
