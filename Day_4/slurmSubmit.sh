#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="matmul"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/path/to/your/home/out/matmul.out.%j
#SBATCH --error=/path/to/your/home/out/matmul.err.%j

export OMP_NUM_THREADS=4
#cd /path/to/bin
./matmul
