#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="pi_speedup"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-01:00:00

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1

####### Output #######
#SBATCH --output=/home/fd0001542/out/pi_speedup.out.%j
#SBATCH --error=/home/fd0001542/out/pi_speedup.err.%j

ulimit -s unlimited
export OMP_STACKSIZE=256M

echo "pi_tasks Speedup Measurements"
echo "Parameters: num_tasks=20000 lower=10000 upper=1000000 seed=42"
echo ""

for threads in 1 2 4 8 16 32 64; do
    echo "Threads: $threads"
    srun ./pi_tasks 20000 $threads 10000 1000000 42
    echo ""
done

echo "Measurements Complete"
