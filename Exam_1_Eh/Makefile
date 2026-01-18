SSH_HOST = fd0001542@hpc.informatik.hs-fulda.de

.PHONY: all clean copy

all: ha

ssh:
	ssh $(SSH_HOST)

clean:
	rm -rf ~/out/*

ha:
	gcc -fopenmp -o heatmap_analysis heatmap_analysis.c
	sbatch ha_submit.sh

copy:
	scp heatmap_analysis.c ha_submit.sh Makefile $(SSH_HOST):~
	exit