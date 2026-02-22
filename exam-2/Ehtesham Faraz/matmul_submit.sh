#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="matmul"

####### Partition #######
#SBATCH --partition=all

####### Resources #######
#SBATCH --time=0-00:10:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=64

####### Output #######
#SBATCH --output=/home/fd0001542/out/matmul.out.%j
#SBATCH --error=/home/fd0001542/out/matmul.err.%j

mpicc -O3 matmul.c -o matmul
mpirun ./matmul 8000 42 0