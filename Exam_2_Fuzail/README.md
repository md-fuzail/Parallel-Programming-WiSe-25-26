# Makefile & Compilation Instructions

This project utilizes a `Makefile`.

## Compiler and Environment (as described in the cluster specifications)

* **Compiler:** `mpicc` (MPI C Compiler)
* **Flags:** `-O3 -lm`
* **Platform:** Fulda HPC Cluster

---

## Makefile Commands

### Compilation

| Command | Description |
| :--- | :--- |
| `make compile` | Compiles the matrix multiplication executable: `matmul`. |
| `make clean` | Removes old output files from the `~/out/` directory on the cluster. |

### Cluster Management

| Command | Description |
| :--- | :--- |
| `make ssh` | Connects to the Fulda HPC cluster via SSH. **Make sure to input your fd number in the SSH_HOST variable in the Makefile** |
| `make copy` | Copies the source file (`matmul.c`), SLURM scripts (`speedup_*.sh`), and the `Makefile` to your home directory on the cluster. |
| `make writeback` | Copies all output files from the cluster's `~/out/` directory back to the `./out/` directory on your local machine. |

### Speedup Experiments (HPC)

These commands submit SLURM batch jobs to measure execution times across different node configurations.

| Command | Description |
| :--- | :--- |
| `make speedup` | Compiles the code and submits **all** speedup experiments at once (1, 2, 4, 6, and 8 nodes). |

---

## Usage Example

To run a full workflow on the cluster:

```bash

# 1. Copy files (run this from your local machine and ensure files are present)
make copy

# 2. Connect to the cluster
make ssh

# 2.1. (optional) Clean your output directory to avoid mixing old logs
make clean

# 3. Compile the program
make compile

# 4. Submit all the node configurations to the queue
make speedup

# 5. (Run from your local machine) Copy the results back once the jobs finish
make writeback