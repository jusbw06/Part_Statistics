#include<stdio.h>

#include"globalVars.h"
#include"model.h"


// GNU PLOT
void plotData(struct node* model, int layer_num){

	const char* name = (char*) "Area Color Plot";
	// open persistent gnuplot window
	FILE* gnuplot_pipe = popen ("gnuplot -persistent", "w"); // <--- Change to temp_file

	// basic settings
	fprintf(gnuplot_pipe, "set title '%s'\n", name);

	// fill it with data
	struct node val;
	FILE* temp_file = fopen64("../tmp/plot_data.txt","w");
	if (temp_file == NULL){
		fprintf(stderr, "Unable to create/open file\n");
		fclose(gnuplot_pipe);
		return;
	}

	for (int k = 0; k <= Z_DIM; k+=10){
		for(int i = 0; i < X_DIM; i++){
			for (int j = 0; j < Y_DIM; j++){
				val = getNode(k, i, j, model);
				if (val.area > 0.1){
					fprintf(temp_file, "%i %i %i %g\n", i, j, k, val.area);
				}
			}
		}
	}
	fclose(temp_file);

	fprintf(gnuplot_pipe, "splot \"../tmp/plot_data.txt\" with points palette\n");

	/*
	int allZeros = 1;
	if (val.area > 0.1)
		allZeros = 0;
	fprintf(stdout,"AllZeros: %d\n", allZeros);
    */

	// refresh can probably be omitted
	fprintf(gnuplot_pipe, "refresh\n");

	fclose(gnuplot_pipe);

}

