#!/usr/bin/env bash

#
# CONFIG
#

THREADS=1

OBJ_CUBE_SIZE_MIN=100
OBJ_CUBE_SIZE_MAX=1600
OBJ_CUBE_SIZE_STEP=20

OMP_PLACES=threads
OMP_PROC_BIND=spread

APP_PATH=${1:-./e_linear_stencil_likwid}

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
  echo "likwid-perfctr $arguments"
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

REGION_IDS=(apply overall)

# Output colors
NORMAL="\\033[0;39m"
RED="\\033[1;31m"
BLUE="\\033[0;34m"
YELLOW="\\033[0;33m"

OUTPUT_FILE="${SCRIPT_ID}_${APP_ID}.csv"

TIME_BENCHMARK_STARTED=$(timestamp)

log_everywhere_silent "benchmark started ($SCRIPT_ID)"

#
# ------------------------------
#

#
# clear file
#
> $OUTPUT_FILE
output "node_id,bench_id,impl_id,region_id,obj_cols,obj_rows,obj_levels,obj_cells,threads,l2d_load_bandwidth,l2d_load_data_volume,l2_bandwidth,l2_data_volume,l3_access_bandwidth,l3_access_data_volumen,l3_access_rate,l3_miss_rate,l3_miss_ratio,memory_bandwidth,memory_data_volume"

#
# ------------------------------
#

#
# EXECUTION
#

OBJ_SIZES=($(echo $(seq $OBJ_CUBE_SIZE_MIN $OBJ_CUBE_SIZE_STEP $OBJ_CUBE_SIZE_MAX)))
RUN_ID=0
RUN_MAX=$(( ${#OBJ_SIZES[@]} * ${#REGION_IDS[@]} * 3))
for OBJ_SIZE in ${OBJ_SIZES[@]}; do
  for REGION_ID in ${REGION_IDS[@]}; do
    OBJ_CELLS=$(( $OBJ_SIZE * $OBJ_SIZE * $OBJ_SIZE ))
    CMD_L2="$(bench_cmd $THREADS L2 $REGION_ID) $APP_PATH $OBJ_SIZE $OBJ_SIZE $OBJ_SIZE"
    log_timestap_everywhere_silent
    log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD_L2"
    OUTPUT_L2=$($CMD_L2)
    RUN_ID=$(( $RUN_ID + 1 ))

    CMD_L3="$(bench_cmd $THREADS L3 $REGION_ID) $APP_PATH $OBJ_SIZE $OBJ_SIZE $OBJ_SIZE"
    log_timestap_everywhere_silent
    log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD_L3"
    OUTPUT_L3=$($CMD_L3)
    RUN_ID=$(( $RUN_ID + 1 ))

    CMD_MEM="$(bench_cmd $THREADS MEM $REGION_ID) $APP_PATH $OBJ_SIZE $OBJ_SIZE $OBJ_SIZE"
    log_timestap_everywhere_silent
    log_everywhere_silent "($(( $RUN_ID + 1 ))/$RUN_MAX) \$ $CMD_MEM"
    OUTPUT_MEM=$($CMD_MEM)
    RUN_ID=$(( $RUN_ID + 1 ))

    #
    # parse results
    #
    IMPL_ID=$(echo "$OUTPUT_L2"              | grep "IMPL_ID_IMPL"                         | cut -d ',' -f 2)

    if [ $THREADS -eq 1 ]; then
      L2D_LOAD_BANDWIDTH=$(echo "$OUTPUT_L2"   | grep "L2D load bandwidth \[MBytes/s\]" | cut -d ',' -f 2)
      L2D_LOAD_DATA_VOLUME=$(echo "$OUTPUT_L2" | grep "L2D load data volume \[GBytes\]" | cut -d ',' -f 2)
      L2_BANDWIDTH=$(echo "$OUTPUT_L2"         | grep "L2 bandwidth \[MBytes/s\]"       | cut -d ',' -f 2)
      L2_DATA_VOLUME=$(echo "$OUTPUT_L2"       | grep "L2 data volume \[GBytes\]"       | cut -d ',' -f 2)

      L3_ACCESS_BANDWIDTH=$(echo "$OUTPUT_L3"    | grep "L3 access bandwidth \[MBytes/s\]" | cut -d ',' -f 2)
      L3_ACCESS_DATA_VOLUMEN=$(echo "$OUTPUT_L3" | grep "L3 access data volume \[GBytes\]" | cut -d ',' -f 2)
      L3_ACCESS_RATE=$(echo "$OUTPUT_L3"         | grep "L3 access rate \[%\]"             | cut -d ',' -f 2)
      L3_MISS_RATE=$(echo "$OUTPUT_L3"           | grep "L3 miss rate \[%\]"               | cut -d ',' -f 2)
      L3_MISS_RATIO=$(echo "$OUTPUT_L3"          | grep "L3 miss ratio \[%\]"              | cut -d ',' -f 2)

      MEMORY_BANDWIDTH=$(echo "$OUTPUT_MEM"   | grep "Memory bandwidth \[MBytes/s\]" | cut -d ',' -f 2)
      MEMORY_DATA_VOLUME=$(echo "$OUTPUT_MEM" | grep "Memory data volume \[GBytes\]" | cut -d ',' -f 2)
    else
      L2D_LOAD_BANDWIDTH=$(echo "$OUTPUT_L2"   | grep "L2D load bandwidth \[MBytes/s\] STAT" | cut -d ',' -f 2)
      L2D_LOAD_DATA_VOLUME=$(echo "$OUTPUT_L2" | grep "L2D load data volume \[GBytes\] STAT" | cut -d ',' -f 2)
      L2_BANDWIDTH=$(echo "$OUTPUT_L2"         | grep "L2 bandwidth \[MBytes/s\] STAT"       | cut -d ',' -f 2)
      L2_DATA_VOLUME=$(echo "$OUTPUT_L2"       | grep "L2 data volume \[GBytes\] STAT"       | cut -d ',' -f 2)

      L3_ACCESS_BANDWIDTH=$(echo "$OUTPUT_L3"    | grep "L3 access bandwidth \[MBytes/s\] STAT" | cut -d ',' -f 2)
      L3_ACCESS_DATA_VOLUMEN=$(echo "$OUTPUT_L3" | grep "L3 access data volume \[GBytes\] STAT" | cut -d ',' -f 2)
      L3_ACCESS_RATE=$(echo "$OUTPUT_L3"         | grep "L3 access rate \[%\] STAT"             | cut -d ',' -f 5)
      L3_MISS_RATE=$(echo "$OUTPUT_L3"           | grep "L3 miss rate \[%\] STAT"               | cut -d ',' -f 5)
      L3_MISS_RATIO=$(echo "$OUTPUT_L3"          | grep "L3 miss ratio \[%\] STAT"              | cut -d ',' -f 5)

      MEMORY_BANDWIDTH=$(echo "$OUTPUT_MEM"   | grep "Memory bandwidth \[MBytes/s\] STAT" | cut -d ',' -f 2)
      MEMORY_DATA_VOLUME=$(echo "$OUTPUT_MEM" | grep "Memory data volume \[GBytes\] STAT" | cut -d ',' -f 2)
    fi

    output "$NODE_ID,$SCRIPT_ID,$IMPL_ID,$REGION_ID,$OBJ_SIZE,$OBJ_SIZE,$OBJ_SIZE,$OBJ_CELLS,$THREADS,$L2D_LOAD_BANDWIDTH,$L2D_LOAD_DATA_VOLUME,$L2_BANDWIDTH,$L2_DATA_VOLUME,$L3_ACCESS_BANDWIDTH,$L3_ACCESS_DATA_VOLUMEN,$L3_ACCESS_RATE,$L3_MISS_RATE,$L3_MISS_RATIO,$MEMORY_BANDWIDTH,$MEMORY_DATA_VOLUME"
  done
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
