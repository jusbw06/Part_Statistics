#include <stdio.h>

#include "globalVars.h"
#include "model_lib.h"
#include "file.h"

// Model Check
void model_printStatistics(struct app_data* data){

	switch (data->model_type){

		case bitArray:
			bitmap_printStatistics(data);
			break;
		case largeFile:
			//here
			break;
		case _default:
			default_printStatistics(data);
			break;
		default:
			fprintf(stderr, "printStatistics: Invalid Model Type\n");

	}


}

// Model Attach
void model_init(struct app_data* data, int create){ // create if part_stat

	switch (data->model_type){

		case bitArray:
			bitmap_init(data, create);
			break;
		case largeFile:
			//here
			break;
		case _default:
			default_init(data, create);
			break;
		default:
			fprintf(stderr, "Init: Invalid Model Type\n");

	}

}

// Model Setup
void model_clear(int model_type, int print_progress){ // create if part_stat

	switch (model_type){
		case bitArray:
			bitmap_clear();
			break;
		case largeFile:
			//here
			break;
		case _default:
			default_clear(print_progress);
			break;
		default:
			fprintf(stderr, "Clear: Invalid Model Type\n");
	}

}

// Model Populate
void model_build(struct app_data* data, int print_progress){

	// launch processes
	default_build(data, print_progress);

}

// k: Z, i: X, j: Y, *val: modified
void model_getVal(int k, int i, int j, void* val, int model_type){

	//k -= (data->start_slice -1);

	uint64_t index;
	switch (model_type){

		case bitArray:
			index = bitmap_getLinearIndex(k, i, j);
			*((uint8_t*) val) = bitmap_getVal(index);
			break;
		case largeFile:
			//here
			break;
		case _default:
			 *((struct node*) val) = default_getNode(k, i, j);
			break;
		default:
			fprintf(stderr, "GetVal: Failed\n");

	}

}

// k: Z, i: X, j: Y
void model_setVal(int k, int i, int j, void* val, int model_type){

	//k -= (data->start_slice -1);

	uint64_t index;
	switch (model_type){

		case bitArray:
			index = bitmap_getLinearIndex(k, i, j);
			bitmap_setVal(index, *((uint8_t*)val));
			break;
		case largeFile:
			//here
			break;
		case _default:
			default_setNode(k, i, j, *((struct node*) val));
			break;
		default:
			fprintf(stderr, "Setval Failed\n");

	}

}

void model_free(struct app_data* data, int destroy){ // destroy if part_stat

	if (destroy)
		destroySharedMem(data->model, data->mem_id);
	else
		detachSharedMem(data->model);

}
