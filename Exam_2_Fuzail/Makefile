SSH_HOST = fd0002114@hpc.informatik.hs-fulda.de

.PHONY: all clean copy ssh compile speedup writeback

all: compile

ssh:
	ssh $(SSH_HOST)

clean:
	rm -rf ~/out/*

compile:
	mpicc -O3 matmulc.c -o matmul -lm

speedup: compile
	sbatch speedup_1node.sh
	sbatch speedup_2nodes.sh
	sbatch speedup_4nodes.sh
	sbatch speedup_6nodes.sh
	sbatch speedup_8nodes.sh

copy:
	scp matmul.c  speedup_*.sh Makefile $(SSH_HOST):~

writeback:
	scp $(SSH_HOST):~/out/* ./out/
