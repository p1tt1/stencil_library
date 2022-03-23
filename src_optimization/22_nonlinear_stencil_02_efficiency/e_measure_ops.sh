#!/usr/bin/env bash

#
# CONFIG
#

APP_PATH=${1:-./e_measure_ops}

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

output() {
  echo $1 >> $OUTPUT_FILE
}

# ------------------------------
#

#
# PROPERTIES
#

SCRIPT_ID=$(filename_without_extension $0)
OUTPUT_FILE="$SCRIPT_ID.csv"

FUNCS=()
OPS=()

#
# ------------------------------
#

#
# clear file
#
> $OUTPUT_FILE
output "func_id,ops"

#
# EXECUTION
#

CMD="likwid-perfctr -C N:0 -f -g FLOPS_DP -m -O ${APP_PATH}"
OUTPUT=$($CMD)

# echo "$OUTPUT"

REGIONS=$(echo "$OUTPUT" | grep "Group 1 Raw")
# echo "$REGIONS"

RETIRED_SSE_AVX_FLOPS_ALL=$(echo "$OUTPUT" | grep "RETIRED_SSE_AVX_FLOPS_ALL")
# echo "$RETIRED_SSE_AVX_FLOPS_ALL"

while IFS= read -r line; do
    # echo $line
    FUNCS+=($(echo "$line" | cut -d ',' -f 2 | cut -d ' ' -f 2))
done <<< "$REGIONS"
# echo ${FUNCS[@]}

while IFS= read -r line; do
    # echo $line
    OPS+=($(echo "$line" | cut -d ',' -f 3))
done <<< "$RETIRED_SSE_AVX_FLOPS_ALL"
# echo ${OPS[@]}

for INDEX in ${!FUNCS[@]}; do
    echo "${FUNCS[$INDEX]}: ${OPS[$INDEX]}"
    output "${FUNCS[$INDEX]},${OPS[$INDEX]}"
done

# #
# # ------------------------------
# #

# #
# # COMPLETED
# #

# exit 0
