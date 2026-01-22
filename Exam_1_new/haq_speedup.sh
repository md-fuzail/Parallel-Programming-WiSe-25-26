#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="haq_speedup"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:15:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/haq_speedup.out.%j
#SBATCH --error=/home/fd0001542/out/haq_speedup.err.%j

echo "heatmap_analysis_quick Speedup Measurements"
echo "Parameters: columns=1024 rows=786 seed=1337 lower=0 upper=100 window_height=20 verbose=0 work_factor=10"
echo ""

for threads in 1 2 4 8 16 32 64; do
    echo "Threads: $threads"
    srun ./heatmap_analysis_quick 1024 786 1337 0 100 20 0 $threads 10
    echo ""
done

echo "Measurements Complete"
