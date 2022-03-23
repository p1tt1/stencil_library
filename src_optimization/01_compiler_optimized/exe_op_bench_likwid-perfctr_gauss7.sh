#!/usr/bin/env bash

print() {
  echo $1 >> $OUTPUT_FILE
}

APP=$1
OUTPUT_FILE="${APP}_likwid-perfctr_gauss7.out"

BENCH_MEM_DP='likwid-perfctr -C S0:1 -f -g MEM_DP'
BENCH_MEM_DP_MARKER='likwid-perfctr -C S0:1 -f -g MEM_DP -m'

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
print "\$ $BENCH_MEM_DP_MARKER $APP"
eval $BENCH_MEM_DP_MARKER $APP >> $OUTPUT_FILE

print "####################################################################"
print "# region whole"
print "####################################################################"
print
print "\$ $BENCH_MEM_DP $APP"
eval $BENCH_MEM_DP $APP >> $OUTPUT_FILE
