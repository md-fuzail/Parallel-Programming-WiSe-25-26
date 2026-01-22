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
echo "Parameters: columns=2048 rows=2048 seed=123 lower=0 upper=100 window_height=1024 verbose=0 work_factor=800"
echo ""

for threads in 1 2 4 8 16 32 64; do
    echo "--- Threads: $threads ---"
    ./heatmap_analysis 2048 2048 123 0 100 1024 0 $threads 800
    echo ""
done

echo "Measurements Complete"
