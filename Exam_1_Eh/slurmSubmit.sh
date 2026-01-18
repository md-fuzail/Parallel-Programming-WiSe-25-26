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
#SBATCH --output=/home/fd0002114/out/heatmap_analysis_optimized.out.%j
#SBATCH --error=/home/fd0002114/out/heatmap_analysis_optimized.err.%j

#cd /path/to/bin
./heatmap_analysis_optimized 1024 786 123 0 100 55 0 1 100