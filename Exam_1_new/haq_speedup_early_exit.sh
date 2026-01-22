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
#SBATCH --output=/home/fd0001542/out/haq_speedup_early_exit.out.%j
#SBATCH --error=/home/fd0001542/out/haq_speedup_early_exit.err.%j

echo "heatmap_analysis_quick Speedup Measurements (Early Exit Enabled)"
echo "Parameters: columns=3 rows=4 seed=42 lower=0 upper=10 window_height=2 verbose=0 work_factor=0"
echo ""

for threads in 1 2 4 8 16 32 64; do
    echo "Threads: $threads"
    export OMP_CANCELLATION=true
    ./heatmap_analysis_quick 3 4 42 0 10 2 1 $threads 0
    echo ""
done

echo "Measurements Complete"
