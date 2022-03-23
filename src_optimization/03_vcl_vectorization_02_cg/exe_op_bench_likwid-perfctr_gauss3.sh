#!/usr/bin/env bash

print() {
  echo $1 >> $OUTPUT_FILE
}

APP=$1
OUTPUT_FILE="${APP}_likwid-perfctr_gauss3.out"

BENCH_MEM='likwid-perfctr -C S0:1 -f -g MEM'
BENCH_MEM_MARKER='likwid-perfctr -C S0:1 -f -g MEM -m'

BENCH_FLOPS_DP='likwid-perfctr -C S0:1 -f -g FLOPS_DP'
BENCH_FLOPS_DP_MARKER='likwid-perfctr -C S0:1 -f -g FLOPS_DP -m'

#
# clear file
#
> $OUTPUT_FILE

print "#"
print "# FACTOR = 1000"
print "# C = 1.0"
print "# H = 1.0"
print "# TAU = 1.0"
print "# RUNS = 1"
print "#"
print

print "####################################################################"
print "# region apply"
print "####################################################################"
print
print "\$ $BENCH_MEM_MARKER $APP"
eval $BENCH_MEM_MARKER $APP >> $OUTPUT_FILE
print "\$ $BENCH_FLOPS_DP_MARKER $APP"
eval $BENCH_FLOPS_DP_MARKER $APP >> $OUTPUT_FILE
