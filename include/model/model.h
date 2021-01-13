#include "globalVars.h"

/*** Model_Check **/
void model_printStatistics(struct app_data* data);

/*** Model_Create ***/
void model_init(struct app_data* data, int create); // create if part_stat

/*** Model_Clear ***/
void model_clear(int model_type, int print_progress);

/*** Model_Build ***/
void model_build(struct app_data* data, int print_progress);

/*** Model_SetGet ***/
void model_getVal(int k, int i, int j, void* val, int model_type);
void model_setVal(int k, int i, int j, void* val, int model_type);

/*** Model_Free **/
void model_free(struct app_data* data, int destroy); // destroy if part_stat
