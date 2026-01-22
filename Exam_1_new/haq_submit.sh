#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="heatmap_analysis_quick"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:05:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/heatmap_analysis_quick.out.%j
#SBATCH --error=/home/fd0001542/out/heatmap_analysis_quick.err.%j

srun ./heatmap_analysis_quick 1024 786 1337 0 100 20 0 1 10
