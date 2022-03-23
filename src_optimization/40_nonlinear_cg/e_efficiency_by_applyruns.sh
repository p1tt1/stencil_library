#!/usr/bin/env bash

#
# CONFIG
#

THREADS=128

APPLY_RUNS_MIN=10
APPLY_RUNS_STEP=10
APPLY_RUNS_MAX=200

OBJ_COLS=1600
OBJ_ROWS=1600
OBJ_LEVELS=1600

OMP_PLACES=threads
OMP_PROC_BIND=spread

# APP_PATH_MEMORYLESS=./e_nonlinear_stencil_mul2
# APP_PATH_PRECALC=./e_nonlinear_stencil_precalc_mul2

# APP_PATH_MEMORYLESS=./e_nonlinear_stencil_pow2
# APP_PATH_PRECALC=./e_nonlinear_stencil_precalc_pow2

# APP_PATH_MEMORYLESS=./e_nonlinear_stencil_exp
# APP_PATH_PRECALC=./e_nonlinear_stencil_precalc_exp

APP_PATH_MEMORYLESS=./e_nonlinear_stencil_pow4_3
APP_PATH_PRECALC=./e_nonlinear_stencil_precalc_pow4_3

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
output "node_id,bench_id,impl_id,func_id,region_id,obj_cols,obj_rows,obj_levels,threads,runs,runtime,efficiency"

#
# ------------------------------
#

#
# EXECUTION
#

#
# NOTE: hardcoded apply_runs=1
#
APPLY_RUNS_LIST=(1 $(echo $(seq $APPLY_RUNS_MIN $APPLY_RUNS_STEP $APPLY_RUNS_MAX)))
RUN_ID=0
RUN_MAX=$(( ${#APPLY_RUNS_LIST[@]} * 2))
for APPLY_RUNS in ${APPLY_RUNS_LIST[@]}; do

  #
  # run precalc
  #

  CMD_PRECALC="$APP_PATH_PRECALC $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $APPLY_RUNS"
  log_timestap_everywhere_silent
  log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD_PRECALC"
  OUTPUT_PRECALC=$(export OMP_NUM_THREADS=$THREADS && export OMP_PLACES=$OMP_PLACES && export OMP_PROC_BIND=$OMP_PROC_BIND && $CMD_PRECALC)
  RUN_ID=$(( $RUN_ID + 1 ))

  IMPL_ID_PRECALC=$(echo "$OUTPUT_PRECALC"         | grep "IMPL_ID_IMPL"         | cut -d ',' -f 2)
  FUNC_ID_PRECALC=$(echo "$OUTPUT_PRECALC"         | grep "FUNC_ID_IMPL"         | cut -d ',' -f 2)
  RUNTIME_APPLY_PRECALC=$(echo "$OUTPUT_PRECALC"   | grep "RUNTIME_APPLY_IMPL"   | cut -d ',' -f 2)
  RUNTIME_OVERALL_PRECALC=$(echo "$OUTPUT_PRECALC" | grep "RUNTIME_OVERALL_IMPL" | cut -d ',' -f 2)
  EFFICIENCY_APPLY_PRECALC="1.0"
  EFFICIENCY_OVERALL_PRECALC="1.0"

  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID_PRECALC,$FUNC_ID_PRECALC,$REGION_ID_APPLY,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$APPLY_RUNS,$RUNTIME_APPLY_PRECALC,$EFFICIENCY_APPLY_PRECALC"
  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID_PRECALC,$FUNC_ID_PRECALC,$REGION_ID_OVERALL,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$APPLY_RUNS,$RUNTIME_OVERALL_PRECALC,$EFFICIENCY_OVERALL_PRECALC"

  log_everywhere_silent "=> Runtime overall precalc: $RUNTIME_OVERALL_PRECALC sec"

  #
  # run memoryless
  #

  CMD_MEMORYLESS="$APP_PATH_MEMORYLESS $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS $APPLY_RUNS"
  log_timestap_everywhere_silent
  log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD_MEMORYLESS"
  OUTPUT_MEMORYLESS=$(export OMP_NUM_THREADS=$THREADS && export OMP_PLACES=$OMP_PLACES && export OMP_PROC_BIND=$OMP_PROC_BIND && $CMD_MEMORYLESS)
  RUN_ID=$(( $RUN_ID + 1 ))

  IMPL_ID_MEMORYLESS=$(echo "$OUTPUT_MEMORYLESS"         | grep "IMPL_ID_IMPL"         | cut -d ',' -f 2)
  FUNC_ID_MEMORYLESS=$(echo "$OUTPUT_MEMORYLESS"         | grep "FUNC_ID_IMPL"         | cut -d ',' -f 2)
  RUNTIME_APPLY_MEMORYLESS=$(echo "$OUTPUT_MEMORYLESS"   | grep "RUNTIME_APPLY_IMPL"   | cut -d ',' -f 2)
  RUNTIME_OVERALL_MEMORYLESS=$(echo "$OUTPUT_MEMORYLESS" | grep "RUNTIME_OVERALL_IMPL" | cut -d ',' -f 2)
  EFFICIENCY_APPLY_MEMORYLESS=$(python3 -c "print($RUNTIME_APPLY_PRECALC/$RUNTIME_APPLY_MEMORYLESS)")
  EFFICIENCY_OVERALL_MEMORYLESS=$(python3 -c "print($RUNTIME_OVERALL_PRECALC/$RUNTIME_OVERALL_MEMORYLESS)")


  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID_MEMORYLESS,$FUNC_ID_MEMORYLESS,$REGION_ID_APPLY,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$APPLY_RUNS,$RUNTIME_APPLY_MEMORYLESS,$EFFICIENCY_APPLY_MEMORYLESS"
  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID_MEMORYLESS,$FUNC_ID_MEMORYLESS,$REGION_ID_OVERALL,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$THREADS,$APPLY_RUNS,$RUNTIME_OVERALL_MEMORYLESS,$EFFICIENCY_OVERALL_MEMORYLESS"

  log_everywhere_silent "=> Runtime overall memoryless: $RUNTIME_OVERALL_MEMORYLESS sec"
  log_everywhere_silent "=> Efficiency overall memoryless: $EFFICIENCY_OVERALL_MEMORYLESS"
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
