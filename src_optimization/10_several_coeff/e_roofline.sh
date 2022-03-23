#!/usr/bin/env bash

#
# CONFIG
#

OBJ_COLS=1024
OBJ_ROWS=1024
OBJ_LEVELS=1024

THREADS=128

APP_PATH=${1:-./e_linear_stencil_several_coeff_likwid}

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

bench_cmd(){
  local arguments="-C $(thread_config $1) -f -g $2 -O"
  if [ $3 = apply ]; then
    arguments="$arguments -m"
  fi
  echo "likwid-perfctr $arguments $APP_PATH"
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
SCRIPT_ID=$(filename_without_extension $0)
REGION_IDS=(apply overall)

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="${SCRIPT_ID}_${APP_ID}.csv"

OBJ_CELLS=$(( $OBJ_COLS * $OBJ_ROWS * $OBJ_LEVELS ))

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_ID)"

#
# clear file
#
> $OUTPUT_FILE
output "node_id,bench_id,impl_id,region_id,obj_cols,obj_rows,obj_levels,obj_cells,threads,mflop_s,mbytes_s"

#
# ------------------------------
#

#
# EXECUTION
#

for region_id in ${REGION_IDS[@]}; do
  CMD_FLOPS_DP="$(bench_cmd $THREADS FLOPS_DP $region_id) $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS"
  log_everywhere_silent "\$ $CMD_FLOPS_DP"
  OUTPUT_FLOPS_DP=$($CMD_FLOPS_DP)
  # echo "$OUTPUT_FLOPS_DP"
  log_timestap_everywhere_silent

  CMD_MEM="$(bench_cmd $THREADS MEM $region_id) $OBJ_COLS $OBJ_ROWS $OBJ_LEVELS"
  log_everywhere_silent "\$ $CMD_MEM"
  OUTPUT_MEM=$($CMD_MEM)
  # echo "$OUTPUT_MEM"
  log_timestap_everywhere_silent

  #
  # parse results
  #

  IMPL_ID=$(echo "$OUTPUT_FLOPS_DP" | grep "IMPL_ID_IMPL"                  | cut -d ',' -f 2)
  MFLOP_S=$(echo "$OUTPUT_FLOPS_DP" | grep "DP \[MFLOP/s\] STAT"                | cut -d ',' -f 2)
  MBYTES_S=$(echo "$OUTPUT_MEM"     | grep "Memory bandwidth \[MBytes/s\] STAT" | cut -d ',' -f 2)

  output "$NODE_ID,$SCRIPT_ID,$IMPL_ID,$region_id,$OBJ_COLS,$OBJ_ROWS,$OBJ_LEVELS,$OBJ_CELLS,$THREADS,$MFLOP_S,$MBYTES_S"
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
