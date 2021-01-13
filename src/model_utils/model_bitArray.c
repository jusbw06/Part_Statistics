#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "globalVars.h"
#include "file.h"

struct bitArrayProperties{

	uint64_t num_elements;
	uint64_t num_bytes;
	uint8_t* bitmap;

	uint64_t resolution; // in microns
	uint64_t xdim;
	uint64_t ydim;
	uint64_t zdim;

};
struct bitArrayProperties bit_ptr;

uint64_t bitmap_getLinearIndex(uint64_t k, uint64_t i, uint64_t j){
	return k * bit_ptr.xdim*bit_ptr.ydim + i*bit_ptr.ydim + j;
}



uint8_t setVal(uint8_t index, uint8_t val, uint8_t set_val){

	uint8_t temp_val;
	if (set_val == 0x01){

		temp_val = set_val << index;
		val = val | temp_val;

	}else if (set_val == 0x00){

		temp_val = 0x01;
		temp_val = ~(temp_val << index);
		val = val & temp_val;

	}

	return val;
}


void bitmap_setVal(uint64_t index, uint8_t set_val){

	uint64_t  byte_index = index / 8;
	uint8_t remainder = index % 8;

	if (set_val > 0x01){
		fprintf(stderr, "Bitmap Set -- Invalid Value: %d\n", set_val);
		return;
	}

	bit_ptr.bitmap[byte_index] = setVal(remainder, bit_ptr.bitmap[byte_index], set_val);

}

uint8_t getVal(uint8_t index, uint8_t val){

	uint8_t temp_val = 0x01;
	temp_val  = temp_val << index;
	temp_val = temp_val & val;

	if (temp_val > 0x00){
			return 0x01;
	}
	return 0x00;

}

uint8_t bitmap_getVal(uint64_t index){


	uint64_t byte_index = index / 8;
	uint8_t remainder = index % 8;

	return getVal(remainder, bit_ptr.bitmap[byte_index]);

}

#define MU_XY 119170
void bitmap_printStatistics(struct app_data* data){

	uint64_t xdim = (uint64_t) ceil((double)2*MU_XY/data->resolution);
	uint64_t ydim = (uint64_t) ceil((double)2*MU_XY/data->resolution);
	uint64_t zdim = (data->end_slice + 1) - data->start_slice;

	uint64_t num_elements = xdim*ydim*zdim;
	uint64_t num_bytes = (uint64_t) ceil((double) num_elements / 8);

	fprintf(stdout, "Model Dimensions(x,y,z): %ld %ld %ld\n", xdim, ydim, zdim);
	fprintf(stdout, "Memory Consumption: %f GB\n", ((double)num_bytes)/1000000000);
	fprintf(stdout, "Model Resolution(xy,z): %d 50\n", data->resolution);
	fflush(stdout);


}

void bitmap_init(struct app_data* data, int create){ // decide dimensions via resolution

	data->Z_DIM = (data->end_slice + 1) - data->start_slice;;
	data->X_DIM = (uint64_t) ceil((double)2*MU_XY/data->resolution);
	data->Y_DIM = (uint64_t) ceil((double)2*MU_XY/data->resolution);

	bit_ptr.xdim = data->X_DIM;
	bit_ptr.ydim = data->Y_DIM;
	bit_ptr.zdim = data->Z_DIM;

	bit_ptr.num_elements = bit_ptr.xdim*bit_ptr.ydim*bit_ptr.zdim;
	bit_ptr.num_bytes = (uint64_t) ceil((double) bit_ptr.num_elements / 8);

	data->mem_size = bit_ptr.num_bytes;

	//fprintf(stderr,"MemSize: %f\n", (double) bit_ptr.num_bytes/1000000000 );

	if (create)
		createSharedMem( (void**) &(bit_ptr.bitmap), &(data->mem_id), data->SHM_KEY, bit_ptr.num_bytes);
	else
		attachToSharedMem( (void**) &(bit_ptr.bitmap), &(data->mem_id), data->SHM_KEY);
	data->model = bit_ptr.bitmap;

}

void bitmap_clear(){
	// clear bitmap
	for (uint64_t i = 0; i < bit_ptr.num_bytes; i++){
		bit_ptr.bitmap[i] = 0;
	}
	fprintf(stdout,"Model Initialized\n");
	fflush(stdout);
}


//void bitmap_build();
//void bitmap_free(struct app_data* data);

/*
void printSizes(uint64_t xdim, uint64_t ydim, uint64_t zdim){

	uint64_t num_elements = xdim*ydim*zdim;
	uint64_t size_bytes = (uint64_t) ceil((double) num_elements / 8);


	fprintf(stdout, "XDIM: %ld\n", xdim);
	fprintf(stdout, "YDIM: %ld\n", ydim);
	fprintf(stdout, "ZDIM: %ld\n", zdim);
	fprintf(stdout, "Num Elements: %ld\n", num_elements);
	fprintf(stdout, "Array Size: %ld bytes (%f GB)\n", size_bytes, (double) size_bytes / 1000000000);


}
*/


/*
int main(int argc, char** argv){

	struct model_dim dims;
	dims.resolution = 7;
	calculate_dim(&dims);

	printSizes(dims.xdim, dims.ydim, dims.zdim);

	struct bitArray array;
	uint64_t num_elements = dims.xdim*dims.ydim*dims.zdim;

	bitmap_init(&array, num_elements);

	for (uint64_t i = 0; i < num_elements; i++){

		bitmap_setVal(i, array.bitmap, i%2);

	}

	uint8_t val;
	for (uint64_t i = 0; i < num_elements; i++){
		val = bitmap_getVal(i, array.bitmap);
		if (val != i%2){
			fprintf(stderr, "Error\n");
			break;
		}
	}
	bitmap_free(&array);

	return 0;

}
*/
