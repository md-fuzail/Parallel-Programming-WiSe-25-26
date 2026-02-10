# Makefile & Compilation Instructions

This project utilizes a `Makefile`.

## Compiler and Environment (as described in the cluster specifications)

* **Compiler:** `gcc/14.3.0`
* **Flags:** `-fopenmp -O3`
* **Platform:** Fulda HPC Cluster

---

## Makefile Commands

### Compilation

| Command | Description |
| :--- | :--- |
| `make compile` | Compiles all three executables: `heatmap_analysis`, `heatmap_analysis_quick`, and `pi_tasks`. |
| `make clean` | Removes old output files. |

### Cluster Management

| Command | Description |
| :--- | :--- |
| `make ssh` | Connects to the Fulda HPC cluster via SSH. **Make sure to input your fd number in the SSH_HOST variable in the make file** |
| `make copy` | Copies the source files and scripts to the cluster. |

### Speedup Experiments (HPC)

These commands submit SLURM batch jobs to measure execution times.

| Command | Description |
| :--- | :--- |
| `make ha_speedup` | Submits the speedup experiment for Task 1.1 (Full Heatmap). |
| `make haq_speedup` | Submits the speedup experiment for Task 1.2 (Heatmap Early Exit). |
| `make pi_speedup` | Submits the speedup experiment for Task 1.3 (Pi Tasks). |
| `make speedup` | Submits **all** speedup experiments at once (Tasks 1.1, 1.2, and 1.3). |

---

## Usage Example

To run a full workflow on the cluster:

```bash

# 1. Copy files (run this from your local machine and ensure files are present)
make copy

# 2. Connect to the cluster
make ssh

# 2.1. (optional) Clean your output directory
make clean

# 3. Compile the programs
make compile

# 4. Submit all the tasks (repeat for desired number (we ran it 10 times) of executions to get the average execution time)
make speedup

# 4.1. (optional) copy the out directory to your host machine
make writeback