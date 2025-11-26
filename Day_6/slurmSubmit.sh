#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="pi_par_loop_red"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0002114/out/pi_par_loop_red.out.%j
#SBATCH --error=/home/fd0002114/out/pi_par_loop_red.err.%j

export OMP_SCHEDULE="dynamic,4"
export OMP_NUM_THREADS=4
#cd /path/to/bin
./pi_par_loop_red
