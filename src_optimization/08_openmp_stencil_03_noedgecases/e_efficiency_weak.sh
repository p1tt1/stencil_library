#!/usr/bin/env bash

#
# CONFIG
#

THREADS_MIN=1
THREADS_MAX=128

OBJ_FACTOR_MIN=100
APP_PATH=$1

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

bench_cmd(){
  local thread_id_max=$(( $1 - 1 ))
  if [ $thread_id_max -eq 0 ]
  then
    local thread_config="N:0"
  else
    local thread_config="N:0-$thread_id_max"
  fi

  echo "likwid-perfctr -C $thread_config -f -g FLOPS_DP -O ${APP_PATH} $2"
}

#
# ------------------------------
#

#
# ENVIRONMENT CHECK
#

if [ $# -eq 0 ]; then
  error_console "No app argument supplied"
  exit 1
fi

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

APP_ID=$(filename $APP_PATH)

NODE_ID=${NODE_ID:-undefined_node}
BENCH_ID="threads"
SCALING="weak"
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
OBJ_FACTOR_LIST=()

#
# ------------------------------
#

#
# GENERATE BENCH LIST WEAK SCALING
#

THREADS=$THREADS_MIN
OBJ_FACTOR=$OBJ_FACTOR_MIN
ITER=1

while [ $THREADS -lt $THREADS_MAX ]
do
  #
  # next bench properties
  #

  ITER=$(( $ITER + 1 ))
  THREADS=$(( $ITER * $THREADS_MIN ))
  OBJ_FACTOR=$(python3 -c "print(round(($ITER ** (1. / 3)) * $OBJ_FACTOR_MIN))")

  THREAD_LIST=($THREADS ${THREAD_LIST[@]})
  OBJ_FACTOR_LIST=($OBJ_FACTOR ${OBJ_FACTOR_LIST[@]})

done

#
# append reference bench
#
THREAD_LIST=($THREADS_MIN ${THREAD_LIST[@]})
OBJ_FACTOR_LIST=($OBJ_FACTOR_MIN ${OBJ_FACTOR_LIST[@]})

#
# clear file
#
> $OUTPUT_FILE
output "node_id,scaling,impl_id,obj_cols,obj_rows,obj_levels,threads,runtime,clock,cpi,flops_dp,efficiency"

#
# ------------------------------
#

#
# EXECUTION
#

for INDEX in ${!THREAD_LIST[@]}; do
  THREADS=${THREAD_LIST[$INDEX]}
  OBJ_FACTOR=${OBJ_FACTOR_LIST[$INDEX]}

  CURRENT_CMD=$(bench_cmd $THREADS $OBJ_FACTOR)

  log_everywhere_silent "($(( $INDEX + 1 ))/${#THREAD_LIST[@]}) \$ $CURRENT_CMD"

  CURRENT_OUTPUT=$($CURRENT_CMD)

  #
  # parse results
  #

  IMPL_ID=$(echo "$CURRENT_OUTPUT"    | grep "IMPL_ID_IMPL"                | cut -d ',' -f 2)
  OBJ_COLS=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_COLS_IMPL"               | cut -d ',' -f 2)
  OBJ_ROWS=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_ROWS_IMPL"               | cut -d ',' -f 2)
  OBJ_LEVELS=$(echo "$CURRENT_OUTPUT" | grep "OBJ_LEVELS_IMPL"             | cut -d ',' -f 2)
  RUNTIME=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_IMPL"                | cut -d ',' -f 2)

  echo "$CURRENT_OUTPUT"
  echo OBJ_COLS: $OBJ_COLS

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
    RUNTIME_REF=$RUNTIME
  fi

  EFFICIENCY=$(python3 -c "print($RUNTIME_REF/$RUNTIME)")

  output "$NODE_ID,$SCALING,$IMPL_ID,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$RUNTIME,$CLOCK,$CPI,$FLOPS_DP,$EFFICIENCY"

  TIME_CURRENT=$(timestamp)
  log_everywhere_silent "=> Runtime: $RUNTIME sec"
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
