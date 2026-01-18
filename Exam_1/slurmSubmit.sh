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
#SBATCH --output=/home/fd0002114/out/pi_tasks_2.out.%j
#SBATCH --error=/home/fd0002114/out/pi_tasks_2.err.%j

#cd /path/to/bin
./pi_tasks_2 1000 4 100000 1000000 42