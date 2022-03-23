#!/usr/bin/env bash

#
# CONFIG
#

THREADS_MIN=1
THREADS_MAX=128

OBJ_COLS=1024
OBJ_ROWS=1024
OBJ_LEVELS=1024

SOLVER_ITER_MAX=1000000

OMP_PLACES=threads
OMP_PROC_BIND=spread

APP_PATH=${1:-./e_cg_b4_hard_part}

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
SCALE=strong
SCRIPT_ID=$(filename_without_extension $0)
SCRIPT_PATH=$(readlink -f $0)
REGION_ID_APPLY='apply'
REGION_ID_ROUTINE='routine'

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="$SCRIPT_ID.csv"

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_PATH)"

THREAD_LIST=()

#
# ------------------------------
#

#
# GENERATE BENCH LIST STRONG SCALING
#

THREADS=$THREADS_MIN
while [ $THREADS -le $THREADS_MAX ]
do
  THREAD_LIST+=($THREADS)
  # THREADS=$(( $THREADS + 1 ))
  THREADS=$(( $THREADS * 2 ))
done

#
# clear file
#
> $OUTPUT_FILE
output "node_id,bench_id,stencil_id,solver_id,region_id,obj_cols,obj_rows,obj_levels,threads,epsilon_solver,iter_max,iter,runtime,efficiency"

#
# ------------------------------
#

#
# EXECUTION
#

for INDEX in ${!THREAD_LIST[@]}; do
  THREADS=${THREAD_LIST[$INDEX]}

  CURRENT_CMD="$APP_PATH $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $SOLVER_ITER_MAX"

  log_everywhere_silent "($(( $INDEX + 1 ))/${#THREAD_LIST[@]}) \$ $CURRENT_CMD"
  CURRENT_OUTPUT=$(export OMP_NUM_THREADS=$THREADS && export OMP_PLACES=$OMP_PLACES && export OMP_PROC_BIND=$OMP_PROC_BIND && $CURRENT_CMD)

  #
  # parse results
  #

  STENCIL_ID=$(echo "$CURRENT_OUTPUT"    | grep "STENCIL_ID_IMPL"                | cut -d ',' -f 2)
  SOLVER_ID=$(echo "$CURRENT_OUTPUT"    | grep "SOLVER_ID_IMPL"                | cut -d ',' -f 2)
  EPSILON_SOLVER=$(echo "$CURRENT_OUTPUT"    | grep "EPSILON_SOLVER_IMPL"                | cut -d ',' -f 2)
  ITER_MAX=$(echo "$CURRENT_OUTPUT"    | grep "ITER_MAX_IMPL"                | cut -d ',' -f 2)
  ITER=$(echo "$CURRENT_OUTPUT"    | grep "ITER_IMPL"                | cut -d ',' -f 2)
  RUNTIME_APPLY=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_APPLY_IMPL"                | cut -d ',' -f 2)
  RUNTIME_ROUTINE=$(echo "$CURRENT_OUTPUT"    | grep "RUNTIME_ROUTINE_IMPL"                | cut -d ',' -f 2)

  if [ $THREADS -eq $THREADS_MIN ]; then
    RUNTIME_REF_APPLY=$RUNTIME_APPLY
    RUNTIME_REF_ROUTINE=$RUNTIME_ROUTINE
  fi

  EFFICIENCY_REL_APPLY=$(python3 -c "print($RUNTIME_REF_APPLY/($THREADS*$RUNTIME_APPLY))")
  EFFICIENCY_REL_ROUTINE=$(python3 -c "print($RUNTIME_REF_ROUTINE/($THREADS*$RUNTIME_ROUTINE))")

  output "$NODE_ID,$SCRIPT_ID,$STENCIL_ID,$SOLVER_ID,$REGION_ID_APPLY,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$EPSILON_SOLVER,$ITER_MAX,$ITER,$RUNTIME_APPLY,$EFFICIENCY_REL_APPLY"
  output "$NODE_ID,$SCRIPT_ID,$STENCIL_ID,$SOLVER_ID,$REGION_ID_ROUTINE,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$EPSILON_SOLVER,$ITER_MAX,$ITER,$RUNTIME_ROUTINE,$EFFICIENCY_REL_ROUTINE"

  TIME_CURRENT=$(timestamp)
  log_everywhere_silent "=> Runtime apply: $RUNTIME_APPLY sec"
  log_everywhere_silent "=> Efficiency rel apply: $EFFICIENCY_REL_APPLY"
  log_everywhere_silent "=> Runtime routine: $RUNTIME_ROUTINE sec"
  log_everywhere_silent "=> Efficiency rel routine: $EFFICIENCY_REL_ROUTINE"
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
