#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="matmul"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=64

####### Output #######
#SBATCH --output=/home/fd0002114/out/matmul.out.%j
#SBATCH --error=/home/fd0002114/out/matmul.err.%j

#cd /path/to/bin
mpirun ./matmul 8000 42 0
