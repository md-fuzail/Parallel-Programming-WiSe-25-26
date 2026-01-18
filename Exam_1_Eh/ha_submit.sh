#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="heatmap_analysis"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/heatmap_analysis.out.%j
#SBATCH --error=/home/fd0001542/out/heatmap_analysis.err.%j

#cd /path/to/bin
srun ./heatmap_analysis 1024 786 123 0 100 55 0 1 100