#include<stdio.h>
#include<libconfig.h>
#include<math.h>

#include"globalVars.h"

#define MU 119170

int refreshValues(config_t* cfg_ptr, int verbose){
	int* globalAddresses[] = {&MAX_THREAD, &X_DIM, &Y_DIM, &Z_DIM, &SHM_KEY, &MODEL_MEM_SIZE_GB, &NUM_SLICES};
	const char* keysv[] = {"MAX_THREAD", "X_DIM", "Y_DIM", "Z_DIM", "SHM_KEY", "MODEL_MEM_SIZE_GB", "NUM_SLICES"};

	for (int i = 0; i < 7; i++){ // <---------- Don't Forget

		if (config_lookup_int(cfg_ptr, keysv[i], globalAddresses[i]) == CONFIG_FALSE){
			fprintf(stderr,"Setting: %s not found\n", "MAX_THREAD");
			config_destroy(cfg_ptr);
			return -1;
		}

	}

	if (verbose){
		fprintf(stdout,"Model Mem Size GB: %d\n", MODEL_MEM_SIZE_GB);
		fprintf(stdout,"Max number of threads: %d\n", MAX_THREAD);
		fprintf(stdout,"Shared Memory Key: 0x%x\n", SHM_KEY);
	}

	/* ALTER X_DIM Y_DIM Z_DIM */
	if (MODEL_MEM_SIZE_GB > 0){
		double x = (double)( ((unsigned long)MODEL_MEM_SIZE_GB) << 30);

		x = x /8;
		x = cbrt(x);

		X_DIM = (int) x;
		Y_DIM = (int) x;
		Z_DIM = (int) (x/4);
	}

	if (verbose){
		fprintf(stdout,"XDIM: %d\n", X_DIM);
		fprintf(stdout,"YDIM: %d\n", Y_DIM);
		fprintf(stdout,"ZDIM: %d\n", Z_DIM);
	}

	MICRONS_PER_SEG = ((double)MU * 2)/X_DIM;

	if (verbose){
		fprintf(stdout,"MICRONS PER SEG: %f\n", MICRONS_PER_SEG);
		fprintf(stdout,"Number of Slices: %d\n", NUM_SLICES);
	}

	return 0;
}


void updateGlobalVariables(const char* filename, int verbose){

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
	if (refreshValues(&cfg, verbose) < 0){
		fprintf(stderr, "Failed to update global variables\n");
	    config_destroy(&cfg);
		return;
	}

	/* Free Memory*/
    config_destroy(&cfg);
}
