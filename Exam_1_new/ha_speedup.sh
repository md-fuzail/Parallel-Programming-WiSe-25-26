#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="ha_speedup"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:15:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/ha_speedup.out.%j
#SBATCH --error=/home/fd0001542/out/ha_speedup.err.%j

echo "heatmap_analysis Speedup Measurements"
echo "Parameters: columns=1024 rows=786 seed=123 lower=0 upper=100 window_height=55 verbose=0 work_factor=100"
echo ""

for threads in 1 2 4 8 16 32 64; do
    echo "--- Threads: $threads ---"
    srun ./heatmap_analysis 1024 786 123 0 100 55 0 $threads 100
    echo ""
done

echo "Measurements Complete"
