#ifndef GLOBALS
#define GLOBALS 1

#ifdef MAIN
/* Global Variables Declaration */
int MAX_THREAD;
int X_DIM;
int Y_DIM;
int Z_DIM;
int SHM_KEY;
int MODEL_MEM_SIZE_GB;
int NUM_SLICES;
double MICRONS_PER_SEG;

#else

/* Global Variables External */
extern int MAX_THREAD;
extern int X_DIM;
extern int Y_DIM;
extern int Z_DIM;
extern int SHM_KEY;
extern int MODEL_MEM_SIZE_GB;
extern int NUM_SLICES;
extern double MICRONS_PER_SEG;

#endif

/* 32 bytes */
struct node{
	double area;
	float area_num_elements;
	double intensity;
	float intensity_num_elements;
	unsigned char tool_paths[8];
	// pos_data here maybe
};




#endif
