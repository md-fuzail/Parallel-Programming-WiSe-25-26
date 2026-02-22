# Makefile & Compilation Instructions

This project utilizes a `Makefile`.

## Compiler and Environment (as described in the cluster specifications)

* **Compiler:** `mpicc` (MPI C compiler)
* **Flags:** `-O3`
* **Platform:** Fulda HPC Cluster

---

## Makefile Commands

### Compilation

| Command | Description |
| :--- | :--- |
| `make compile` | Compiles the `matmul` executable from `matmul.c`. |
| `make clean` | Removes old output files from the cluster. |

### Cluster Management

| Command | Description |
| :--- | :--- |
| `make ssh` | Connects to the Fulda HPC cluster via SSH. **Make sure to input your fd number in the SSH_HOST variable in the Makefile** |
| `make copy` | Copies the source files (`matmul.c`, `matmul_submit.sh`, `Makefile`) to the cluster. |
| `make writeback` | Downloads output files from the cluster to the local `out/` directory. |

### Job Submission (HPC)

| Command | Description |
| :--- | :--- |
| `make matmul` | Compiles the program and submits a SLURM batch job to run the matrix multiplication. |

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

# 3. Compile and submit the job
make matmul

# 3.1. (optional) Download output files to your local machine
make writeback
```

## Program Usage

The `matmul` program requires three command-line arguments:

```bash
./matmul <n> <seed> <verbose>
```

**Arguments:**
- `n`: Matrix dimension (n × n matrices)
- `seed`: Random number generator seed for reproducibility
- `verbose`: Set to `1` to print matrices (only for n ≤ 10), `0` otherwise

The default job submission script (`matmul_submit.sh`) runs:
```bash
./matmul 8000 42 0
```

This computes 8000×8000 matrix multiplication with seed 42 and verbose mode disabled.

## Output

The program outputs:
- **Checksum**: Sum of all elements in the result matrix (for verification)
- **Execution Time**: Time taken to complete the computation with the number of MPI ranks used

If verbose mode is enabled and n ≤ 10, it also prints matrices A, B, and C.

## Project Structure

```
parallel_mpi/
├── matmul.c              # Main MPI matrix multiplication implementation
├── Makefile              # Build and deployment automation
├── matmul_submit.sh      # SLURM job submission script
├── matmul                # Compiled executable (generated)
└── out/                  # Output directory for job results
```