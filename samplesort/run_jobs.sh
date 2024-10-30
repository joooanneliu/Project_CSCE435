#!/bin/bash

array_sizes=(16)
num_procs=(2 4 8 16 32 64 128 256 512)
#num_procs=(1024)
#num_procs=(128 256 512)
arr_type=("Sorted" "Reverse_sorted" "Random" "1_perturbed")

for sz in "${array_sizes[@]}"; do
    for proc_sz in "${num_procs[@]}"; do
        for type in "${arr_type[@]}"; do
            echo sbatch mpi$proc_sz.grace_job $sz $proc_sz $type
            sbatch mpi$proc_sz.grace_job $sz $proc_sz $type
        done
    done
done