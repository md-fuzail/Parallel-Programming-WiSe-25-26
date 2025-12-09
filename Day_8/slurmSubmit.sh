#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="fib_tasks"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0002114/out/fib_tasks.out.%j
#SBATCH --error=/home/fd0002114/out/fib_tasks.err.%j

export OMP_NUM_THREADS=4
#cd /path/to/bin
./fib_tasks