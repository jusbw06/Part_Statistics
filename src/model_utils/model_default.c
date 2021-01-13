#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#include "globalVars.h"
#include "file.h"

struct defaultModelProperties{

	uint64_t num_elements;
	uint64_t size;
	struct node* model;

	uint64_t resolution; // in microns
	uint64_t xdim;
	uint64_t ydim;
	uint64_t zdim;

};
struct defaultModelProperties model_ptr;


uint64_t default_getLinearIndex(uint64_t k, uint64_t i, uint64_t j){
	return k * model_ptr.xdim*model_ptr.ydim + i*model_ptr.ydim + j;
}


// k: Z, i: X, j: Y
struct node default_getNode(int k, int i, int j){
	uint64_t index = default_getLinearIndex(k,i,j);
	return model_ptr.model[index];
}

// k: Z, i: X, j: Y
void default_setNode(int k, int i, int j, struct node val){
	uint64_t index = default_getLinearIndex(k,i,j);
	model_ptr.model[index] = val;
}

#define MU_XY 119170
void default_printStatistics(struct app_data* data){

	/* From MAX_MEM */
	double x = (double)( ((unsigned long) data->MODEL_MEM_SIZE_GB) << 30);

	x = x /sizeof(struct node)*4;
	x = cbrt(x);

	uint64_t xdim  = (uint64_t) x;
	uint64_t ydim  = (uint64_t) x;
	uint64_t zdim = (uint64_t) x/4;

	uint64_t num_bytes = xdim*ydim*zdim*sizeof(struct node);
	uint64_t resolution = ((double)MU_XY * 2)/xdim;


	fprintf(stdout, "Model Dimensions(x,y,z): %ld %ld %ld\n", xdim, ydim, zdim);
	fprintf(stdout, "Memory Consumption: %f GB\n", ((double)num_bytes)/1000000000);
	fprintf(stdout, "Model Resolution(xyz): %ld\n", resolution);
	fflush(stdout);


}


void default_init(struct app_data* data, int create){ // decide dimensions via MODEL_MEM_SIZE

	/* From MAX_MEM */
	double x = (double)( ((unsigned long) data->MODEL_MEM_SIZE_GB) << 30);

	x = x /sizeof(struct node)*4;
	x = cbrt(x);

	data->X_DIM = (int) x;
	data->Y_DIM = (int) x;
	data->Z_DIM = (int) x/4;

	model_ptr.size = (uint64_t)(data->X_DIM) * (uint64_t)(data->Y_DIM) * (uint64_t)(data->Z_DIM) * sizeof(struct node);
	model_ptr.xdim = data->X_DIM;
	model_ptr.ydim = data->Y_DIM;
	model_ptr.zdim = data->Z_DIM;

	//fprintf(stderr, "Dim: %ld %ld %ld\nSize: %ld\n", model_ptr.xdim, model_ptr.ydim, model_ptr.zdim, model_ptr.size);

	data->mem_size = model_ptr.size;
	model_ptr.resolution = ((double)MU_XY * 2)/(data->X_DIM);
	data->MICRONS_PER_SEG = model_ptr.resolution;

	// Allow user to confirm Here

	if (create){
		createSharedMem( (void **) &(model_ptr.model), &(data->mem_id), data->SHM_KEY, model_ptr.size);
	}else{
		attachToSharedMem( (void **) &(model_ptr.model), &(data->mem_id), data->SHM_KEY);
	}
	data->model = model_ptr.model;
}

void default_clear(int print_progress){

	if(print_progress){
		fprintf(stdout,"Model Initialization Percentage:\n");
		fflush(stdout);
		fprintf(stdout,"\r%d %%", 0);
		fflush(stdout);
	}

	struct node val;
	val.area = 0;
	val.intensity = 0;
	val.num_elements = 0;

	for (uint64_t k = 0; k < model_ptr.zdim; k++){
		for (uint64_t i = 0; i < model_ptr.xdim; i++){
			for (uint64_t j = 0; j < model_ptr.ydim; j++){

				default_setNode(k,i,j, val);

			}

		}
		if (print_progress){
			fprintf(stdout,"\r%d %%", (int)(((double)k)/model_ptr.zdim * 100) );
			fflush(stdout);
		}
	}
	if (print_progress){
		fprintf(stdout,"\r%d %%", 100 );
		fflush(stdout);
	}
}

void default_build(struct app_data* data, int print_progress){

	if(print_progress){
		fprintf(stdout,"Data Imported Percentage:\n");
		fflush(stdout);
		fprintf(stdout,"\r%d %%", 0);
		fflush(stdout);
	}

	char input_num[10];
	char input_enum[10];
	int wstatus;
	int num_processes = 0;
	int child_pid = 0;
	int pid = 0;
	int num_slices = data->end_slice - data->start_slice;
	int current_layer = data->start_slice;


	while(current_layer <= data->end_slice){
		while (num_processes < data->MAX_THREAD && current_layer <= data->end_slice){

			/* Read TDMS file & Build from Layer & Free Channel Data */
			// launch new process
			sprintf(input_num,"%d", current_layer);

			sprintf(input_enum,"%d", data->model_type);

			pid = fork();
			if (pid == 0){ // isChild
				//fprintf(stderr, "Launched: %s\n", input_num);
				if (execl("tdms_readlayer","tdms_readlayer",input_num, input_enum, NULL) == -1){ // add model_type argument
					fprintf(stderr, "Child: Exec Error, Exiting...\n");
					exit(-1);
				}
			}
			num_processes++;
			current_layer++;
		}

		child_pid = wait(&wstatus);
		if ( child_pid > 0 ){
			num_processes--;
		}

		if (print_progress){
			fprintf(stdout,"\r%d %%", (int)( ((double)current_layer - data->start_slice) / num_slices * 100));
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
		fflush(stdout);
	}

}

//void default_free();
