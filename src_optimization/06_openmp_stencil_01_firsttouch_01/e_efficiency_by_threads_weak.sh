#!/usr/bin/env bash

#
# CONFIG
#

THREADS_MIN=1
THREADS_MAX=128

OBJ_FACTOR_MIN=51
VEC_SIZE=4

OMP_PLACES=threads
OMP_PROC_BIND=spread

APP_PATH=${1:-./e_linear_stencil}

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

filename_without_extension() {
  local name=$(filename $0)
  echo "${name%.*}"
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

APP_ID=$(filename $APP_PATH)

NODE_ID=${NODE_ID:-undefined_node}
SCALE=weak
SCRIPT_ID=$(filename_without_extension $0)
REGION_ID_APPLY='apply'
REGION_ID_ROUTINE='overall'

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="${SCRIPT_ID}_${APP_ID}.csv"

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_ID)"

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
  OBJ_FACTOR=$(python3 -c "print($VEC_SIZE * round(($ITER ** (1. / 3)) * $OBJ_FACTOR_MIN))")

  THREAD_LIST=($THREADS ${THREAD_LIST[@]})
  OBJ_FACTOR_LIST=($OBJ_FACTOR ${OBJ_FACTOR_LIST[@]})

done

#
# append reference bench
#
THREAD_LIST=($THREADS_MIN ${THREAD_LIST[@]})
OBJ_FACTOR_LIST=($(( $VEC_SIZE * $OBJ_FACTOR_MIN )) ${OBJ_FACTOR_LIST[@]})

# echo ${THREAD_LIST[@]}
echo ${OBJ_FACTOR_LIST[@]}

# #
# # clear file
# #
# > $OUTPUT_FILE
# output "node_id,bench_id,scale,impl_id,region_id,obj_cols,obj_rows,obj_levels,threads,runtime,efficiency"

# #
# # ------------------------------
# #

# #
# # EXECUTION
# #

# for INDEX in ${!THREAD_LIST[@]}; do
#   THREADS=${THREAD_LIST[$INDEX]}
#   OBJ_SIZE=$(( ${OBJ_FACTOR_LIST[$INDEX]} + 2 ))

#   CURRENT_CMD="$APP_PATH $OBJ_SIZE $OBJ_SIZE $OBJ_SIZE"

#   log_everywhere_silent "($(( $INDEX + 1 ))/${#THREAD_LIST[@]}) \$ $CURRENT_CMD"
#   CURRENT_OUTPUT=$(export OMP_NUM_THREADS=$THREADS && export OMP_PLACES=$OMP_PLACES && export OMP_PROC_BIND=$OMP_PROC_BIND && $CURRENT_CMD)

#   #
#   # parse results
#   #

#   IMPL_ID=$(echo "$CURRENT_OUTPUT"    | grep "IMPL_ID_IMPL"                | cut -d ',' -f 2)
#   OBJ_COLS_RESULT=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_COLS_IMPL"               | cut -d ',' -f 2)
#   OBJ_ROWS_RESULT=$(echo "$CURRENT_OUTPUT"   | grep "OBJ_ROWS_IMPL"               | cut -d ',' -f 2)
#   OBJ_LEVELS_RESULT=$(echo "$CURRENT_OUTPUT" | grep "OBJ_LEVELS_IMPL"             | cut -d ',' -f 2)
#   RUNTIME_APPLY=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_APPLY_IMPL"                | cut -d ',' -f 2)
#   RUNTIME_ROUTINE=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_ROUTINE_IMPL"                | cut -d ',' -f 2)

#   if [ $THREADS -eq $THREADS_MIN ]; then
#     RUNTIME_REF_APPLY=$RUNTIME_APPLY
#     RUNTIME_REF_ROUTINE=$RUNTIME_ROUTINE
#   fi

#   EFFICIENCY_APPLY=$(python3 -c "print($RUNTIME_REF_APPLY/($THREADS*$RUNTIME_APPLY))")
#   EFFICIENCY_ROUTINE=$(python3 -c "print($RUNTIME_REF_ROUTINE/($THREADS*$RUNTIME_ROUTINE))")

#   output "$NODE_ID,$SCRIPT_ID,$SCALE,$IMPL_ID,$REGION_ID_APPLY,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$THREADS,$RUNTIME_APPLY,$EFFICIENCY_APPLY"
#   output "$NODE_ID,$SCRIPT_ID,$SCALE,$IMPL_ID,$REGION_ID_ROUTINE,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$THREADS,$RUNTIME_ROUTINE,$EFFICIENCY_ROUTINE"

#   TIME_CURRENT=$(timestamp)
#   log_everywhere_silent "=> Runtime apply: $RUNTIME_APPLY sec"
#   log_everywhere_silent "=> Efficiency apply: $EFFICIENCY_APPLY"
#   log_everywhere_silent "=> Runtime routine: $RUNTIME_ROUTINE sec"
#   log_everywhere_silent "=> Efficiency routine: $EFFICIENCY_ROUTINE"
#   log_everywhere_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
#   log_everywhere_silent "------------------"

# done

# #
# # ------------------------------
# #

# #
# # COMPLETED
# #

# TIME_CURRENT=$(timestamp)
# log_everywhere_silent "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $TIME_CURRENT)"
# log_everywhere "benchmark finished"
# log_everywhere_silent "=================="

# exit 0
