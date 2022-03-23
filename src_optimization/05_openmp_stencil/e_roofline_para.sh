#!/usr/bin/env bash


OUTPUT="DP [MFLOP/s],1805.2779,1666.2942,492.9313,606.3280,2010.1769,1388.5898,1119.8517,232.8955,2025.3691,2627.5145,3517.3077,4141.1075,2170.9660,3079.4368,3833.0576,3676.2700,1979.1679,1168.5269,530.3435,579.4984,2296.2579,1681.8871,1064.8381,649.3680,2060.6397,2894.0304,3629.1054,4040.5602,2194.2659,3099.9024,3939.9846,3944.4845,317.1030,324.6439,365.2023,355.2749,307.3382,270.7401,316.8334,316.8959,676.7986,658.1917,676.9074,650.5993,644.6269,273.4662,236.1592,271.5325,659.5423,659.9645,300.8637,591.8960,585.0610,700.5047,690.3158,235.9385,237.3223,663.0022,268.5171,676.8140,671.9300,358.0675,669.4774,236.6611,648.6726,237.2133,233.0940,238.8483,758.3588,752.9869,236.3836,236.9383,4263.1622,4196.9935,3792.9583,3618.9777,4263.2530,4178.8028,3729.5071,4156.0315,768.0232,760.9963,237.1987,233.0612,236.7040,657.7658,237.2058,666.6968,4102.2337,4175.4202,3817.1060,3635.9848,4043.7477,4215.2210,3612.0181,3654.1743,281.0732,279.6924,317.7242,774.9606,318.8377,317.2169,319.0896,306.0074,659.6884,649.0209,235.7913,237.4133,233.4592,697.1986,233.2440,272.6998,680.9799,678.7382,233.8508,639.9902,680.2599,685.3027,647.8457,702.5625,643.5614,666.8045,693.2247,262.3384,647.5975,656.4158,248.2434,659.0634,"
echo $OUTPUT
echo ---

VALUES=$(echo $OUTPUT | cut -c 14- | rev | cut -c 2- | rev)
echo $VALUES
echo ---

MEAN=$(python3 -c "import statistics; print(statistics.mean([$VALUES]))")
MEDIAN=$(python3 -c "import statistics; print(statistics.median([$VALUES]))")
FLOP_S_MEDIAN_SCALED=$(echo "$MEDIAN*128" | bc)

echo mean:$MEAN
echo median:$MEDIAN
echo median:$FLOP_S_MEDIAN_SCALED

echo ---






# #
# # CONFIG
# #

# OBJ_CUBE_SIZE_INNER=2048

# THREADS=128

# APP_PATH=${1:-./e_linear_stencil_para_likwid}

# ENABLE_TELEGRAM=true

# #
# # ------------------------------
# #

# #
# # FUNCTIONS
# #

# filename() {
#   echo "$(basename $(readlink -f $1))"
# }

# log_console() {
#   echo -e "$BLUE $1 $NORMAL"
# }
# warning_console() {
#   echo -e "$YELLOW > WARNING - $1 $NORMAL"
# }

# error_console() {
#   echo ""
#   echo -e "$RED >>> ERROR - $1 $NORMAL" >&2
# }

# output() {
#   echo $1 >> $OUTPUT_FILE
# }

# log_telegram() {
#   if [ "${ENABLE_TELEGRAM}" = true ]; then
#     curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}" >/dev/null
#   fi
# }

# log_telegram_silent() {
#   if [ "${ENABLE_TELEGRAM}" = true ]; then
#     curl -s "https://api.telegram.org/bot${TELEGRAM_BOT_TOKEN}/sendMessage?chat_id=${TELEGRAM_CHAT_ID}&text=${1}&disable_notification=true" >/dev/null
#   fi
# }

# log_everywhere() {
#   log_console "$1"
#   log_telegram "$1"
# }

# log_everywhere_silent() {
#   log_console "$1"
#   log_telegram_silent "$1"
# }

# log_timestap_everywhere() {
#   local log_cmd=${1:-log_everywhere}
#   local time_current=$(timestamp)
#   $log_cmd "$(elapsedTimeMessage $TIME_BENCHMARK_STARTED $time_current)"
# }

# log_timestap_everywhere_silent() {
#   log_timestap_everywhere log_everywhere_silent
# }

# timestamp() {
#   date +%s
# }

# elapsedTimeMessage() {
#   local l_elapsed=$(( $2 - $1 ))
#   eval "echo Elapsed time: $(date -ud "@$l_elapsed" +'$((%s/3600/24)) days %H hr %M min %S sec')"
# }

# thread_config(){
#   local thread_id_max=$(( $1 - 1 ))
#   if [ $thread_id_max -eq 0 ]
#   then
#     local config="N:0"
#   else
#     local config="N:0-$thread_id_max"
#   fi
#   echo $config
# }

# bench_cmd(){
#   local arguments="-C $(thread_config $1) -f -g $2 -O"
#   if [ $3 = apply ]; then
#     arguments="$arguments -m"
#   fi
#   echo "likwid-perfctr $arguments"
# }

# #
# # ------------------------------
# #

# #
# # ENVIRONMENT CHECK
# #

# if [ -z ${TELEGRAM_BOT_TOKEN} ]; then
#   error_console "TELEGRAM_BOT_TOKEN isn't defined."
#   exit 1
# fi
# if [ -z ${TELEGRAM_CHAT_ID} ]; then
#   error_console "TELEGRAM_CHAT_ID isn't defined."
#   exit 1
# fi

# #
# # ------------------------------
# #

# #
# # PROPERTIES
# #

# APP_ID=$(filename $APP_PATH)

# NODE_ID=${NODE_ID:-undefined_node}
# BENCH_ID="roofline"
# SCRIPT_PATH=$(readlink -f $0)
# REGION_IDS=(apply overall)

# BENCH_NUM=$((${#TILINGS_COLS[@]} * ${#TILINGS_LEVELS_ROWS[@]}))

# # Output colors
# NORMAL="\\033[0;39m"
# RED="\\033[1;31m"
# BLUE="\\033[0;34m"
# YELLOW="\\033[0;33m"

# OUTPUT_FILE="${APP_ID}_${BENCH_ID}_${NODE_ID}.csv"

# TIME_BENCHMARK_STARTED=$(timestamp)

# log_everywhere_silent "benchmark started ($SCRIPT_PATH)"

# #
# # clear file
# #
# > $OUTPUT_FILE
# output "node_id,bench_id,impl_id,region_id,threads,obj_cols,obj_rows,obj_levels,mflop_s,mbytes_s"

# #
# # ------------------------------
# #

# #
# # EXECUTION
# #

# for region_id in ${REGION_IDS[@]}; do
#   CMD_FLOPS_DP="$(bench_cmd $THREADS FLOPS_DP $region_id) $APP_PATH $OBJ_CUBE_SIZE_INNER"
#   log_everywhere_silent "\$ $CMD_FLOPS_DP"
#   OUTPUT_FLOPS_DP=$($CMD_FLOPS_DP)
#   echo "$OUTPUT_FLOPS_DP"
#   log_timestap_everywhere_silent

#   CMD_MEM="$(bench_cmd $THREADS MEM $region_id) $APP_PATH $OBJ_CUBE_SIZE_INNER"
#   log_everywhere_silent "\$ $CMD_MEM"
#   OUTPUT_MEM=$($CMD_MEM)
#   # echo "$OUTPUT_MEM"
#   log_timestap_everywhere_silent

#   #
#   # parse results
#   #

#   IMPL_ID=$(echo "$OUTPUT_FLOPS_DP" | grep "IMPL_ID_IMPL"                       | cut -d ',' -f 2)
#   OBJ_COLS_RESULT=$(echo "$OUTPUT_FLOPS_DP"   | grep "OBJ_COLS_IMPL"               | cut -d ',' -f 2)
#   OBJ_ROWS_RESULT=$(echo "$OUTPUT_FLOPS_DP"   | grep "OBJ_ROWS_IMPL"               | cut -d ',' -f 2)
#   OBJ_LEVELS_RESULT=$(echo "$OUTPUT_FLOPS_DP" | grep "OBJ_LEVELS_IMPL"             | cut -d ',' -f 2)
#   MFLOP_S=$(echo "$OUTPUT_FLOPS_DP" | grep "DP \[MFLOP/s\] STAT"                | cut -d ',' -f 2)
#   MBYTES_S=$(echo "$OUTPUT_MEM"     | grep "Memory bandwidth \[MBytes/s\] STAT" | cut -d ',' -f 2)

#   output "$NODE_ID,$BENCH_ID,$IMPL_ID,$region_id,$THREADS,$OBJ_COLS_RESULT,$OBJ_ROWS_RESULT,$OBJ_LEVELS_RESULT,$MFLOP_S,$MBYTES_S"
# done

# #
# # ------------------------------
# #

# #
# # COMPLETED
# #

# log_everywhere "benchmark finished"
# log_everywhere_silent "=================="

# exit 0
