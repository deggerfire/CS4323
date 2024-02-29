#!/usr/bin/env bash
#
# This file builds the verifier code based on the configured vars.
# You should not need to modify this, but feel free to!
#
# - richard.m.veras@ou.edu

# Turn on command echo for debugging 
set -x

source op_dispatch_vars.sh

echo $OP_BASELINE_FILE

echo $OP_SUBMISSION_VAR01_FILE
echo $CC
echo $CFLAGS

# NOTE: Should move these out
COMPUTE_NAME_REF="baseline"
DISTRIBUTED_ALLOCATE_NAME_REF="baseline_allocate"
DISTRIBUTED_FREE_NAME_REF="baseline_free"
DISTRIBUTE_DATA_NAME_REF="baseline_distribute"
COLLECT_DATA_NAME_REF="baseline_collect"

COMPUTE_NAME_TST="test"
DISTRIBUTED_ALLOCATE_NAME_TST="test_allocate"
DISTRIBUTED_FREE_NAME_TST="test_free"
DISTRIBUTE_DATA_NAME_TST="test_distribute"
COLLECT_DATA_NAME_TST="test_collect"

TEST_RIG="timer_op.c"

# Build the timer
# NOTE: need gnu99/gnu11 to get the POSIX compliance for timing
${CC} -std=gnu99 -O2 -c \
    -DCOMPUTE_NAME_REF=${COMPUTE_NAME_REF} \
    -DDISTRIBUTED_ALLOCATE_NAME_REF=${DISTRIBUTED_ALLOCATE_NAME_REF} \
    -DDISTRIBUTED_FREE_NAME_REF=${DISTRIBUTED_FREE_NAME_REF} \
    -DDISTRIBUTE_DATA_NAME_REF=${DISTRIBUTE_DATA_NAME_REF} \
    -DCOLLECT_DATA_NAME_REF=${COLLECT_DATA_NAME_REF} \
    -DCOMPUTE_NAME_TST=${COMPUTE_NAME_TST} \
    -DDISTRIBUTED_ALLOCATE_NAME_TST=${DISTRIBUTED_ALLOCATE_NAME_TST} \
    -DDISTRIBUTED_FREE_NAME_TST=${DISTRIBUTED_FREE_NAME_TST} \
    -DDISTRIBUTE_DATA_NAME_TST=${DISTRIBUTE_DATA_NAME_TST} \
    -DCOLLECT_DATA_NAME_TST=${COLLECT_DATA_NAME_TST} \
    ${TEST_RIG} -static -fPIC -o ${TEST_RIG}.o


# build the variants

${CC} $CFLAGS -c \
    -DCOMPUTE_NAME=${COMPUTE_NAME_TST} \
    -DDISTRIBUTE_DATA_NAME=${DISTRIBUTE_DATA_NAME_TST} \
    -DCOLLECT_DATA_NAME=${COLLECT_DATA_NAME_TST} \
    -DDISTRIBUTED_ALLOCATE_NAME=${DISTRIBUTED_ALLOCATE_NAME_TST}\
    -DDISTRIBUTED_FREE_NAME=${DISTRIBUTED_FREE_NAME_TST}\
    ${OP_SUBMISSION_VAR01_FILE} -o ${OP_SUBMISSION_VAR01_FILE}.o

# build the timers
${CC} ${CFLAGS} ${TEST_RIG}.o ${OP_SUBMISSION_VAR01_FILE}.o -o ./run_bench_op_var01.x
