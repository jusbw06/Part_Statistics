#ifndef GLOBALS
#define GLOBALS 1

#include <stdint.h>

/* 20 bytes */
struct __attribute__ ((packed)) node{
	double area;
	double intensity;
	float num_elements;
};


struct app_data{

	/* CONFIGURATION CONSTANTS */
	int MAX_THREAD;
	int X_DIM;
	int Y_DIM;
	int Z_DIM;
	int SHM_KEY;
	int MODEL_MEM_SIZE_GB;
	int NUM_SLICES;
	double MICRONS_PER_SEG;

	/* Important Variables */
	void* model;
	int mem_id;
	unsigned long mem_size;
	int start_slice;
	int end_slice;

	int resolution; //microns
	int model_type; // init in main
	int MODEL_HDD_SIZE_GB;
	char tdms_dir[100];
	char filename[100];

	void* gui_data;
};


enum model_type{ bitArray=0, largeFile=1, _default=2 };


#endif
