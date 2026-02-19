#!/usr/bin/env bash
####### Mail Notify / Job Name / Comment #######
#SBATCH --job-name="matmul_1N"

####### Partition #######
#SBATCH --partition=all

####### Ressources #######
#SBATCH --time=0-00:15:00     

####### Node Info #######
#SBATCH --exclusive
#SBATCH --nodes=1              
#SBATCH --ntasks-per-node=64

####### Output #######
#SBATCH --output=/home/fd0002114/out/matmul_1N.out.%j
#SBATCH --error=/home/fd0002114/out/matmul_1N.err.%j

cd /home/fd0002114/Parallel-Programming-WiSe-25-26/Exam_2_Fuzail

echo "=================================================="
echo "Starting Speedup Benchmark on $SLURM_JOB_NUM_NODES Nodes"
echo "Matrix Size: 8000, Seed: 42, Verbose: 0"
echo "=================================================="

for i in {1..5}
do
    echo "--- Run $i ---"
    mpirun ./matmul 8000 42 0
    echo ""
done

echo "Benchmark Complete."