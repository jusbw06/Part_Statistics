#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>

#include"globalVars.h"

// k: Z, i: X, j: Y
struct node getNode(int k, int i, int j, struct node* model){

	unsigned long index = k * X_DIM*Y_DIM + i*Y_DIM + j;
	return model[index];

}

// k: Z, i: X, j: Y
void setNode(int k, int i, int j, struct node* model, struct node val){

	unsigned long index = k * X_DIM*Y_DIM + i*Y_DIM + j;
	model[index] = val;

}


void initModel(int print_progress, struct node* model){


	if(print_progress){
		fprintf(stdout,"Model Initialization Percentage:\n");
		fprintf(stdout,"\r%d %%", 0);
		fflush(stdout);
	}

	struct node val;
	val.area = 0;
	val.area_num_elements = 0;
	val.intensity = 0;
	val.intensity_num_elements = 0;
	val.tool_paths[0] = 0;


	for (int k = 0; k < Z_DIM; k++){
		for (int i = 0; i < X_DIM; i++){
			for (int j = 0; j < Y_DIM; j++){

				setNode(k,i,j, model, val);

			}

		}
		if (print_progress){
			fprintf(stdout,"\r%d %%", (int)(((double)k)/Z_DIM * 100) );
			fflush(stdout);
		}
	}
	if (print_progress){
		fprintf(stdout,"\r%d %%", 100 );
		fprintf(stdout,"\n");
	}
}

void buildModel(int print_progress){

	if(print_progress){
		fprintf(stdout,"Data Imported Percentage:\n");
		fprintf(stdout,"\r%d %%", 0);
		fflush(stdout);
	}

	char input_num[10];
	int wstatus;
	int num_processes = 0;
	int child_pid = 0;
	int total_processes = 0;
	int pid = 0;

	while(total_processes < NUM_SLICES){
		while (num_processes < MAX_THREAD && total_processes < NUM_SLICES){

			/* Read TDMS file & Build from Layer & Free Channel Data */
			// launch new process
			sprintf(input_num,"%d",total_processes+1);

			pid = fork();
			if (pid == 0){ // isChild
				//fprintf(stderr, "Launched: %s\n", input_num);
				if (execl("tdms_readlayer","tdms_readlayer",input_num, NULL) == -1){
					fprintf(stderr, "Child: Exec Error, Exiting...\n");
					exit(-1);
				}
			}
			num_processes++;
			total_processes++;
		}

		child_pid = wait(&wstatus);
		if ( child_pid > 0 ){
			num_processes--;
		}

		if (print_progress){
			fprintf(stdout,"\r%d %%", (int)( ((double)total_processes) / NUM_SLICES * 100));
			fflush(stdout);
		}

	}
	/* wait for remaining children */
	while (num_processes > 0){
		child_pid = wait(&wstatus);
		if ( child_pid > 0 ){
			num_processes--;
		}
	}
	if(print_progress){
		fprintf(stdout,"\r%d %%", 100 );
		fprintf(stdout,"\n");
	}

}
