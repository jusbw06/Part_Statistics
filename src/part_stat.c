#define MAIN 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "globalVars.h"
#include "plotting.h"
#include "settings.h"
#include "file.h"
#include "model.h"
#include <fcntl.h>
#include <unistd.h>

int ack_fd;
int interactive_mode;


void removeQuotes(char* str){

	int i = 0;
	int j = 0;
	while (str[i] != '\0'){

		if (str[i] != '"'){
			str[i] = str[j];
			j++;
		}
		i++;
	}
	str[j] = '\0';
}


void updateVariable(struct app_data* data){

	char* var_name = strtok(NULL, " ");
	char* var_val = strtok(NULL, "\n");

	if (var_name == NULL || var_val == NULL){
		fprintf(stderr, "updateVariable: invalid syntax");
		return;
	}

	if ( strcmp(var_name,"start_slice") == 0){
		data->start_slice = atoi(var_val);
	}else if ( strcmp(var_name,"end_slice") == 0){
		data->end_slice = atoi(var_val);
	}else if ( strcmp(var_name,"model_type") == 0){
		data->model_type = atoi(var_val);
	}else if ( strcmp(var_name,"resolution") == 0 || strcmp(var_name,"res") == 0){
		data->resolution = atoi(var_val);
	}else if ( strcmp(var_name,"mem_max") == 0 ){
		data->MODEL_MEM_SIZE_GB = atoi(var_val);
	}else if ( strcmp(var_name,"hdd_max") == 0 ){
		data->MODEL_HDD_SIZE_GB = atoi(var_val);
	}else if ( strcmp(var_name,"tdms_dir") == 0 ){
		//removeQuotes(var_val);
		strcpy(data->tdms_dir,var_val);
	}else if ( strcmp(var_name,"filename") == 0 ){
		//removeQuotes(var_val);
		strcpy(data->filename,var_val);
	}else{
		fprintf(stderr, "Invalid Var Name: %s\n", var_name);
		return;
	}

	fprintf(stdout,"Variable Updated: %s %s\n", var_name, var_val);
	fflush(stdout);

}


void launch_interpretter(int argc, char** argv){

	struct app_data data;

	data.model_type = _default;
	data.resolution = 50;


	//strcpy(data.tdms_dir,"");

	int verbose = 1;
	/* Initial Routine */
	fprintf(stdout,"Welcome to Part_Stat:\n");
	fflush(stdout);
	updateGlobalVariables("../SETTINGS.cfg", &data, 0);
	//fprintf(stdout,"Size of [must be 20 bytes] node: %ld bytes\n", sizeof(struct node));

	/* Interpreter */
	if (interactive_mode){
		fprintf(stdout,"\nWelcome to the interactive prompt:\n");
		fprintf(stdout, "-->  ");
		fflush(stdout);
	}

	char* token;
	char line_buffer[100];
	while (fgets(line_buffer, 100, stdin) != NULL){
		token = strtok(line_buffer, " \n");

		if (token == NULL){
			continue;
		}


		if (strcmp(token,"create_model") == 0){
			model_init(&data, 1); // create if part_stat
			fprintf(stdout,"Memory Allocated\n");
			fflush(stdout);
			if (!interactive_mode)
				write(ack_fd, "OK\n", 3);
		}else if (strcmp(token,"clear_model") == 0){
			model_clear(data.model_type, 1);
		}else if (strcmp(token,"build_model") == 0){
			model_build(&data, 1);
		}else if ( strcmp(token,"free_model") == 0 ){
			model_free(&data, 1); // destroy if part_stat
		}else if (strcmp(token, "exit") == 0){
			fprintf(stdout,"Exitting\n");
			fflush(stdout);
			if (!interactive_mode)
				write(ack_fd, "OK\n", 3);
			return;
		}else if ( strcmp(token,"plot") == 0 ){
			plotData(&data, 0);
		}else if ( strcmp(token, "save_model") == 0){ // Check For Model COmpat Next
			saveToFile(data.model, data.mem_size, 1);
		}else if ( strcmp(token, "load_model") == 0){
			readFromFile(data.model, data.mem_size, 1);
		}else if ( strcmp(token, "refresh_settings") == 0){
			updateGlobalVariables("../SETTINGS.cfg", &data, 0);
		}else if ( strcmp(token, "print_model_stats") == 0){
			model_printStatistics(&data);
		}else if(strcmp(token, "update_variable") == 0){
			updateVariable(&data);
			// update from here
		}else{
			fprintf(stderr,"Unrecognized Option: %s\n", token);
		}
		fprintf(stdout, "Action: %s\n", token);
		fflush(stdout);
		if (interactive_mode){
			fprintf(stdout, "-->  ");
			fflush(stdout);
		}

	}

}



int main(int argc, char** argv){

	interactive_mode = 1;
	/*if (argc > 1){
		if ( strcmp(argv[1], "--gui") == 0){
			launch_gui(argc, argv);
			return 0;
		}
	}*/

	if (argc > 1){
		interactive_mode = 0;
		ack_fd = atoi(argv[1]);
	}


	launch_interpretter(argc, argv);

	if (!interactive_mode)
		close(ack_fd);

	return 0;

}
