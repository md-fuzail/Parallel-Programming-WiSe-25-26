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

ha:
	gcc -fopenmp -O3 -o heatmap_analysis heatmap_analysis.c
	sbatch ha_submit.sh

haq:
	gcc -fopenmp -O3 -o heatmap_analysis_quick heatmap_analysis_quick.c
	sbatch haq_submit.sh

pi:
	gcc -fopenmp -O3 -o pi_tasks pi_tasks.c
	sbatch pi_submit.sh

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
	sbatch pi_speedup.sh

copy:
	scp heatmap_analysis.c heatmap_analysis_quick.c pi_tasks.c \
	    ha_submit.sh haq_submit.sh pi_submit.sh \
	    ha_speedup.sh haq_speedup.sh pi_speedup.sh \
	    Makefile $(SSH_HOST):~
