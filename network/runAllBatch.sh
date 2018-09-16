#!/bin/bash

sbatch network-TCP-1000-1thread.slurm
sbatch network-TCP-1000-2thread.slurm
sbatch network-TCP-1000-4thread.slurm
sbatch network-TCP-1000-8thread.slurm
sbatch network-TCP-32000-1thread.slurm
sbatch network-TCP-32000-2thread.slurm
sbatch network-TCP-32000-4thread.slurm
sbatch network-TCP-32000-8thread.slurm

sbatch network-UDP-1000-1thread.slurm
sbatch network-UDP-1000-2thread.slurm
sbatch network-UDP-1000-4thread.slurm
sbatch network-UDP-1000-8thread.slurm
sbatch network-UDP-32000-1thread.slurm
sbatch network-UDP-32000-2thread.slurm
sbatch network-UDP-32000-4thread.slurm
sbatch network-UDP-32000-8thread.slurm

sbatch network-TCP-1-1thread.slurm
sbatch network-TCP-1-2thread.slurm
sbatch network-TCP-1-4thread.slurm
sbatch network-TCP-1-8thread.slurm

sbatch network-UDP-1-1thread.slurm
sbatch network-UDP-1-2thread.slurm
sbatch network-UDP-1-4thread.slurm
sbatch network-UDP-1-8thread.slurm

