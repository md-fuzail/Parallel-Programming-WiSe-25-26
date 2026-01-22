#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="pi_tasks"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/pi_tasks.out.%j
#SBATCH --error=/home/fd0001542/out/pi_tasks.err.%j

ulimit -s unlimited
export OMP_STACKSIZE=256M
srun ./pi_tasks 20000 4 10000 1000000 42