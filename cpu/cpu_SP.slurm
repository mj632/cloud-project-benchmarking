#!/bin/bash
#SBATCH --nodes=1
#SBATCH --output=./output/cpu_SP_1thread.out.dat
#SBATCH --wait-all-nodes=1

./cpu_SP.sh $SLURM_JOB_NODELIST
