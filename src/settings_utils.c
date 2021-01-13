#include<stdio.h>
#include<libconfig.h>
#include<math.h>

#include"globalVars.h"

#define MU 119170

#define ADDR(x) &(data->x)
int refreshValues(config_t* cfg_ptr, struct app_data* data, int verbose){
	int* globalAddresses[] = { ADDR(MAX_THREAD), ADDR(X_DIM), ADDR(Y_DIM), ADDR(Z_DIM), ADDR(SHM_KEY), ADDR(MODEL_MEM_SIZE_GB), ADDR(NUM_SLICES)};
	const char* keysv[] = {"MAX_THREAD", "X_DIM", "Y_DIM", "Z_DIM", "SHM_KEY", "MODEL_MEM_SIZE_GB", "NUM_SLICES"};

	for (int i = 0; i < 7; i++){ // <---------- Don't Forget

		if (config_lookup_int(cfg_ptr, keysv[i], globalAddresses[i]) == CONFIG_FALSE){
			fprintf(stderr,"Setting: %s not found\n", "MAX_THREAD");
			config_destroy(cfg_ptr);
			return -1;
		}

	}

	if (verbose){
		fprintf(stdout,"Model Mem Size GB: %d\n", data->MODEL_MEM_SIZE_GB);
		fprintf(stdout,"Max number of threads: %d\n", data->MAX_THREAD);
		fprintf(stdout,"Shared Memory Key: 0x%x\n", data->SHM_KEY);
	}

	/* ALTER X_DIM Y_DIM Z_DIM */
	if (data->MODEL_MEM_SIZE_GB > 0){
		double x = (double)( ((unsigned long)data->MODEL_MEM_SIZE_GB) << 30);

		x = x /sizeof(struct node)*4;
		x = cbrt(x);

		data->X_DIM = (int) x;
		data->Y_DIM = (int) x;
		data->Z_DIM = (int) (x/4);
	}

	if (verbose){
		fprintf(stdout,"XDIM: %d\n", data->X_DIM);
		fprintf(stdout,"YDIM: %d\n", data->Y_DIM);
		fprintf(stdout,"ZDIM: %d\n", data->Z_DIM);
	}

	data->MICRONS_PER_SEG = ((double)MU * 2)/(data->X_DIM);

	if (verbose){
		fprintf(stdout,"MICRONS PER SEG: %f\n", data->MICRONS_PER_SEG);
		fprintf(stdout,"Number of Slices: %d\n", data->NUM_SLICES);
	}

	data->start_slice = 1;
	data->end_slice = data->NUM_SLICES;

	return 0;
}


void updateGlobalVariables(const char* filename, struct app_data* data, int verbose){

	config_t cfg;
	//config_setting_t *setting;

	/* Initialize config object*/
	config_init (&cfg);

	/* Read config object from file*/
	if ( config_read_file(&cfg, filename) == CONFIG_FALSE ){
		fprintf(stderr,"Error opening config file: %s\n", filename);
		fprintf(stderr, "Error Information: %s\n", config_error_text(&cfg));
	    config_destroy(&cfg);
		return;
	}

	/* Update global Variables*/
	if (refreshValues(&cfg, data, verbose) < 0){
		fprintf(stderr, "Failed to update global variables\n");
	    config_destroy(&cfg);
		return;
	}

	/* Free Memory*/
    config_destroy(&cfg);
}
