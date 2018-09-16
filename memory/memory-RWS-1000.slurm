#!/bin/bash
#SBATCH --nodes=1
#SBATCH --output=./output/memory-RWR-1-1thread.out.dat
#SBATCH --wait-all-nodes=1

./memory-RWS-1000.sh $SLURM_JOB_NODELIST
