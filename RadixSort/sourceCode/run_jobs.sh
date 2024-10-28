#!/bin/bash

array_sizes=(26)
num_procs=(4 8 16 32 64 128 256 512)
#num_procs=(1024)
#num_procs=(128 256 512)
arr_type=("Sorted" "Reverse_sorted" "Random" "1_perturbed")

for sz in "${array_sizes[@]}"; do
    for proc_sz in "${num_procs[@]}"; do
        for type in "${arr_type[@]}"; do
            arr_sz=$((2**sz))
            echo sbatch mpi.grace_job$proc_sz $arr_sz $proc_sz $type
            sbatch mpi.grace_job$proc_sz $arr_sz $proc_sz $type
        done
    done
done
