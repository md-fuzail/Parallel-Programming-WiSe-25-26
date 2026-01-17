#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="hello_world_ring"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=4

####### Output #######
#SBATCH --output=/home/fdai0329/out/hello_world_ring.out.%j
#SBATCH --error=/home/fdai0329/out/hello_world_ring.err.%j

#For Jonas:
cd /home/fdai0329/WiSe2526ParaProgCode/09/out
mpirun ./hello_world_ring
