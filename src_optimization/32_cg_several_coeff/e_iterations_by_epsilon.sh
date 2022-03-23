#!/usr/bin/env bash

#
# CONFIG
#

THREADS=128

EPS_E10_MIN=0
EPS_E10_STEP=1
EPS_E10_MAX=40

OBJ_COLS=512
OBJ_ROWS=512
OBJ_LEVELS=512

OMP_PLACES=threads
OMP_PROC_BIND=spread

APP_PATH=${1:-./e_cg_stencil}

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

log_timestap_everywhere() {
  local log_cmd=${1:-log_everywhere}
  local time_current=$(timestamp)
  $log_cmd "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $time_current)"
}

log_timestap_everywhere_silent() {
  log_timestap_everywhere log_everywhere_silent
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

NODE_ID=${NODE_ID:-undefined_node}
SCALE=strong
SCRIPT_ID=$(filename_without_extension $0)
REGION_ID_APPLY='apply'
REGION_ID_OVERALL='overall'

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="$SCRIPT_ID.csv"

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_ID)"

#
# clear file
#
> $OUTPUT_FILE
output "node_id,bench_id,impl_id,region_id,obj_cols,obj_rows,obj_levels,threads,epsilon,iter_max,runtime,iterations"

#
# ------------------------------
#

#
# EXECUTION
#

#
# NOTE: hardcoded apply_runs=1
#
EPS_E10_LIST=($(echo $(seq $EPS_E10_MIN $EPS_E10_STEP $EPS_E10_MAX)))
RUN_ID=0
RUN_MAX=${#EPS_E10_LIST[@]}
for EPS_E10 in ${EPS_E10_LIST[@]}; do

  #
  # run precalc
  #

  CMD="$APP_PATH $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $EPS_E10"
  log_timestap_everywhere_silent
  log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD"
  OUTPUT=$(export OMP_NUM_THREADS=$THREADS && export OMP_PLACES=$OMP_PLACES && export OMP_PROC_BIND=$OMP_PROC_BIND && $CMD)
  RUN_ID=$(( $RUN_ID + 1 ))

  IMPL_ID=$(echo "$OUTPUT"         | grep "IMPL_ID_IMPL"         | cut -d ',' -f 2)
  RUNTIME_APPLY=$(echo "$OUTPUT"   | grep "RUNTIME_APPLY_IMPL"   | cut -d ',' -f 2)
  EPSILON=$(echo "$OUTPUT"         | grep "EPSILON_IMPL"         | cut -d ',' -f 2)
  ITER_MAX=$(echo "$OUTPUT"        | grep "ITER_MAX_IMPL"        | cut -d ',' -f 2)
  ITER=$(echo "$OUTPUT"            | grep "ITER_IMPL"            | cut -d ',' -f 2)
  RUNTIME_OVERALL=$(echo "$OUTPUT" | grep "RUNTIME_OVERALL_IMPL" | cut -d ',' -f 2)

  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID,$REGION_ID_APPLY,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$EPSILON,$ITER_MAX,$RUNTIME_APPLY,$ITER"
  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID,$REGION_ID_APPLY,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$EPSILON,$ITER_MAX,$RUNTIME_OVERALL,$ITER"

  log_everywhere_silent "=> Runtime overall: $RUNTIME_OVERALL sec"
  log_everywhere_silent "EPSILON: $EPSILON"
  log_everywhere_silent "ITER: $ITER"
  log_everywhere_silent "------------------"

done

#
# ------------------------------
#

#
# COMPLETED
#

log_everywhere "benchmark finished"
log_timestap_everywhere_silent
log_everywhere_silent "=================="

exit 0
