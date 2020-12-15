#define MAIN 1

#include<stdio.h>
#include<string.h>
#include <stdlib.h>

#include"globalVars.h"
#include"plotting.h"
#include"settings.h"
#include"file.h"
#include"model.h"

int main(int argc, char** argv){

	struct node* model = NULL;
	int mem_id;
	int verbose = 1;


	unsigned long mem_size;
	/* Initial Routine */
	fprintf(stdout,"Welcome to Part_Stat. Your current settings are shown below:\n\n");
	updateGlobalVariables("../SETTINGS.cfg", 1);
	mem_size = X_DIM*Y_DIM*Z_DIM*sizeof(struct node);

	/* Interpreter */
	fprintf(stdout,"\nWelcome to the interactive prompt:\n");
	fprintf(stdout, "-->  ");
	char* token;
	char line_buffer[100];
	while (fgets(line_buffer, 100, stdin) != NULL){
		token = strtok(line_buffer, " \n");

		if (strcmp(token,"init_mem") == 0){
			// init mem
			createSharedMem( (void**) &model, &mem_id, mem_size );
		}else if (strcmp(token,"zero_model") == 0){
			// init model; Set Model to Zeros
			initModel(verbose, model);
		}else if (strcmp(token,"build_model") == 0){
			// Build Model
			buildModel(verbose);
		}else if ( strcmp(token,"exit") == 0 ){
			//destroy shmem
			destroySharedMem(model, mem_id);
			return 0;
		}else if ( strcmp(token,"plot") == 0 ){
			//token = strtok(NULL, " \n");
			//plot the data
			//int layer = atoi(token);
			plotData(model, 0);
		}else if ( strcmp(token, "save_model") == 0){
			//print the data to file
			saveToFile(model, mem_size, 1);
		}else if ( strcmp(token, "load_model") == 0){
			//load memory from file
			readFromFile(model, mem_size, 1);
		}else if ( strcmp(token, "refresh_settings") == 0){
			//load memory from file
			updateGlobalVariables("../SETTINGS.cfg", 1);
			mem_size = X_DIM*Y_DIM*Z_DIM*sizeof(struct node);
		}else{
			fprintf(stderr,"Unrecognized Option: %s\n", token);
		}
		fprintf(stdout, "-->  ");
		fflush(stdout);
	}


	return 0;

}
