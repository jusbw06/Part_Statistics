#define MAIN 1

#include "TdmsParser.h"
#include "TdmsGroup.h"
#include "TdmsChannel.h"
#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "globalVars.h"
#include "model.h"
#include "settings.h"
#include "file.h"


void appendToLoc(int x_loc, int y_loc, int z_loc, double area, double intensity, double ratio, struct app_data* data){

	if (data->model_type == largeFile || data->model_type == _default){

		struct node val;
		model_getVal(z_loc, x_loc, y_loc, &val, data->model_type); //

		val.area += area * ratio;
		val.intensity += intensity * ratio;
		val.num_elements += ratio;

		model_setVal(z_loc, x_loc, y_loc, &val, data->model_type); //
	}

	if (data->model_type == bitArray){

		uint8_t one = 1;
		model_setVal(z_loc, x_loc, y_loc, &one, data->model_type); //

	}

}

#define AREA 0
#define X_AXIS 1
#define Y_AXIS 2
#define PARAMETER 3
#define INTENSITY 4

#define MU 119170
void interpolateAndAppend(double x, double y, double z, double area, double intensity, struct app_data* data){

	int x_grid_loc = int( (double) (x + MU) / data->MICRONS_PER_SEG);
	int y_grid_loc = int( (double) (y + MU) / data->MICRONS_PER_SEG);
	int z_grid_loc = int( (double) z / data->MICRONS_PER_SEG);

	double x_rem = (double) (x + MU) / data->MICRONS_PER_SEG - x_grid_loc;
	double y_rem = (double) (y + MU) / data->MICRONS_PER_SEG - y_grid_loc;
	double z_rem = (double) z / data->MICRONS_PER_SEG - z_grid_loc;
	// remainder should be % of 1 already

	int xs[8] = {0, 0, 1, 1, 0, 0, 1, 1};
	int ys[8] = {0, 1, 0, 1, 0, 1, 0, 1};
	int zs[8] = {0, 0, 0, 0, 1, 1, 1, 1};

	for (int i = 0; i < 8; i++){
		double x_rat = 0, y_rat = 0, z_rat = 0;

		if (xs[i] == 0)
			x_rat = (1-x_rem);
		else
			x_rat = x_rem;

		if (ys[i] == 0)
			y_rat = (1-y_rem);
		else
			y_rat = y_rem;

		if (zs[i] == 0)
			z_rat = (1-z_rem);
		else
			z_rat = z_rem;

		double vol = x_rat*y_rat*z_rat; // from [0, 1]
		appendToLoc(x_grid_loc + xs[i], y_grid_loc + ys[i], z_grid_loc + zs[i], area, intensity, vol, data);


	}

	// add toolpath data later

}

// ------> parallelize this <--------
void buildFromLayer(int slice_num, int* verbose, struct app_data* data){

	char file_name[100];
	int storeValues = true;

	sprintf(file_name,"/run/media/justi/Ext 2/Numerical Docs Only/1/Slice%05i.tdms", slice_num);
//	sprintf(file_name,"Slice%05i.tdms", slice_num); TODO


	TdmsParser parser(file_name, storeValues);
	if (parser.fileOpeningError()){
		fprintf(stderr,"\nError opening Slice%05i.tdms\n", slice_num);
		return;
	}


	if (verbose[1]){
		fprintf(stderr,"File Slice%05i.tdms opened\n", slice_num);
		fprintf(stderr,"Beginning File Read: Slice%05i.tdms\n", slice_num);
	}

	/* Reads Document, Creates Data Structures */
	parser.read(false); // verbose

	if (verbose[1]){
		fprintf(stderr,"Finished File Read: Slice%05i.tdms\n", slice_num);
		fprintf(stderr,"Beginning Data Import\n");
	}

	std::vector<double> data_area, data_x, data_y, data_int;

	//int z_grid_loc = int( (double) slice_num * 50 / MICRONS_PER_SEG);
	//int y_grid_loc = int( (double) (MU + MU) / MICRONS_PER_SEG);
	//fprintf(stderr, "z_grid_loc: %d y_grid_loc: %d\n", z_grid_loc, y_grid_loc);

	unsigned int num_groups = parser.getGroupCount();
	TdmsChannel* ch[5];
	for (int i = 0; i < int(num_groups); i++){

		/* Extract Channel Data */
		TdmsGroup* curr_group = parser.getGroup( i );

		//unsigned int num_channels = curr_group->getGroupSize();
		ch[AREA] = curr_group->getChannel( int(AREA) );
		ch[X_AXIS] = curr_group->getChannel( int(X_AXIS));
		ch[Y_AXIS]  = curr_group->getChannel( int(Y_AXIS));
		ch[INTENSITY]  = curr_group->getChannel( int(INTENSITY));

		data_area = ch[AREA]->getDataVector();
		data_x = ch[X_AXIS]->getDataVector();
		data_y = ch[Y_AXIS]->getDataVector();
		data_int = ch[INTENSITY]->getDataVector();


		if (verbose[2]){
			fprintf(stderr,"Appending Data to Model Group [%i]\n", i);
		}

		/* Append to Model */
		unsigned long long num_elements = ch[AREA]->getDataCount();
		for (int m = 1; m < 5; m++){
			if ( (m != PARAMETER) && (ch[m]->getDataCount() < num_elements) ){
				num_elements = ch[m]->getDataCount();
			}
		}


		for (int n = 0; n < int(num_elements); n++){

			// if invalid skip
			if (data_area.at(n) <= 0.1){
				continue;
			}

			// interp here
			interpolateAndAppend(data_x.at(n) * 6, data_y.at(n) * 6, slice_num * 50, data_area.at(n), data_int.at(n), data);

		}

	}
	if (verbose[1]){
		fprintf(stderr,"Finished Data Import\n");
	}

	/* Free Memory */
	parser.freeMemory();
	parser.close();

}


int main(int argc, char *argv[]){


	struct app_data data;
	int slice_num;
	int model_type;

	if (argc < 2){
		fprintf(stderr,"Child: Missing Slice Number Argument\n");
		return 1;
	}

	if (argc < 3){
		fprintf(stderr,"Child: Missing Model Type Argument\n");
		return 1;
	}

	slice_num = atoi(argv[1]);
	if (slice_num == 0){
		fprintf(stderr, "Child: Invalid Slice Number\n");
		return 1;
	}
	model_type = atoi(argv[2]);
	if (model_type < 0 || model_type > 2){
		fprintf(stderr, "Child: Invalid Model Type\n");
		return 1;
	}
	data.model_type = model_type;
	data.resolution = 50;

	//fprintf(stderr, "model_type: %d", data.model_type);

	updateGlobalVariables("../SETTINGS.cfg", &data, 0);
	model_init(&data, 0); // create if part_stat

	// Read And Append Values
	int verbosity[] = {0,0,0};
	buildFromLayer(slice_num, verbosity, &data);


	/* Detach Shared Memory */
	model_free(&data, 0);


	return 0;
}

