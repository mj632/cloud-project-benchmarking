#!/bin/bash
#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --output=./output/network-TCP-1-1thread.out.dat
#SBATCH --wait-all-nodes=1

srun ./network-TCP-32000-1thread.sh $SLURM_JOB_NODELIST

