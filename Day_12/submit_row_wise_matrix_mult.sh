#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="row_wise_matrix_mult"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=64

####### Output #######
#SBATCH --output=/home/fd0002114/out/row_wise_matrix_mult.out.%j
#SBATCH --error=/home/fd0002114/out/row_wise_matrix_mult.err.%j

#cd /path/to/bin
mpirun ./row_wise_matrix_mult
