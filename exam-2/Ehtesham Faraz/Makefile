SSH_HOST = fd0001542@hpc.informatik.hs-fulda.de

.PHONY: all clean copy ssh matmul

all: compile

ssh:
	ssh $(SSH_HOST)

clean:
	rm -rf ~/out/*

compile:
	mpicc -O3 matmul.c -o matmul

matmul:
	mpicc -O3 matmul.c -o matmul
	sbatch matmul_submit.sh

copy:
	scp matmul.c matmul_submit.sh \
	    Makefile $(SSH_HOST):~

writeback:
	scp $(SSH_HOST):~/out/* ./out/
