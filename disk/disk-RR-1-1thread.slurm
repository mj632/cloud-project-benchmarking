#!/bin/bash
#SBATCH --nodes=1
#SBATCH --output=./output/disk-RR-1-1thread.out.dat
#SBATCH --wait-all-nodes=1

./disk-RR-1-1thread.sh $SLURM_JOB_NODELIST
