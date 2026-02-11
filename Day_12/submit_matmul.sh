#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="hello_world"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=4

####### Output #######
#SBATCH --output=/home/fd0002114/out/hello_world.out.%j
#SBATCH --error=/home/fd0002114/out/hello_world.err.%j

#cd /path/to/bin
mpirun -np 4 ./matmul 4 42 1
