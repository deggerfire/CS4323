# This Makefile orchestrates the building, verification and timing of
# your implementations.
#
# - richard.m.veras@ou.edu
#
#
# Run modes:
# 1. execute on machine with mpi already installed and loaded.
#    Do not use this on schooner, only gpel and your machine:
#    make all-local
#
# 2. If you are on a local system with mpi loaded through module, then
#    you can run locally on that node for debugging by typing. For the
#    most part this will probably only apply to Dr. Veras:
#
#    module load OpenMPI
#    make all-local
#
#
# 3. execute remotely on Schooner.
#    make all-schooner
#

# Increments to use in the local tests
MIN=16
MAX=512
STEP=16


# This is just for local runs
NUMRANKS=1

# You should not need to adjust this
MPIVER_SCHOONER="OpenMPI/4.1.1-GCC-11.2.0"
MPIVER="mpi/mpich-x86_64"


# NOTE: https://stackoverflow.com/questions/32217413/error-while-trying-to-load-module-environment-in-makefile
# make will not find "module" because by default it uses "sh" and not "bash"
SHELL:=/bin/bash
# Additionally, we have to use ":\" for each line that we want to use the module environment


clean:
	rm -f *.x *~ *.o

cleanall: clean
	rm -f *.csv *.png
	rm baseline_op.c
	rm tuned_variant*

all-schooner: build-verifier-schooner  build-bench-schooner 
	sbatch parallel-prob.sbatch

build-verifier-schooner:
	module load ${MPIVER_SCHOONER}; \
	./build_test_op.sh

build-bench-schooner:
	module load ${MPIVER_SCHOONER}; \
	./build_bench_op.sh

all-local: run-verifier-local run-bench-local

run-verifier-local: build-verifier-local
	mpiexec -n ${NUMRANKS} ./run_test_op_var01.x  ${MIN} ${MAX} ${STEP} 1 result_verification_local_op_var01.csv
	cat result_verification_local_op_var01.csv
	echo "Number of FAILS: `grep "FAIL" result_verification_local_op_*.csv|wc -l`"

run-bench-local: build-bench-local
	mpiexec -n ${NUMRANKS} ./run_bench_op_var01.x ${MIN} ${MAX} ${STEP} 1 result_bench_local_op_var01.csv
	cat result_bench_local_op_var01.csv

build-verifier-local:
	./build_test_op.sh

build-bench-local:
	./build_bench_op.sh

