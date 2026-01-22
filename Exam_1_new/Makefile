SSH_HOST = fd0001542@hpc.informatik.hs-fulda.de

.PHONY: all clean copy ssh ha haq pi speedup ha_speedup haq_speedup pi_speedup

all: compile

ssh:
	ssh $(SSH_HOST)

clean:
	rm -rf ~/out/*

compile:
	gcc -fopenmp -O3 -o heatmap_analysis heatmap_analysis.c
	gcc -fopenmp -O3 -o heatmap_analysis_quick heatmap_analysis_quick.c
	gcc -fopenmp -O3 -o pi_tasks pi_tasks.c

ha_speedup:
	gcc -fopenmp -O3 -o heatmap_analysis heatmap_analysis.c
	sbatch ha_speedup.sh

haq_speedup:
	gcc -fopenmp -O3 -o heatmap_analysis_quick heatmap_analysis_quick.c
	sbatch haq_speedup.sh

pi_speedup:
	gcc -fopenmp -O3 -o pi_tasks pi_tasks.c
	sbatch pi_speedup.sh

speedup: compile
	sbatch ha_speedup.sh
	sbatch haq_speedup.sh
	sbatch haq_speedup_early_exit.sh
	sbatch pi_speedup.sh

copy:
	scp heatmap_analysis.c heatmap_analysis_quick.c pi_tasks.c \
	    ha_speedup.sh haq_speedup.sh haq_speedup_early_exit.sh pi_speedup.sh \
	    Makefile $(SSH_HOST):~

writeback:
	scp $(SSH_HOST):~/out/* ./out/
