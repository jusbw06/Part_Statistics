#include <stdint.h>
#include "globalVars.h"

/* BitArray Functions */
uint64_t bitmap_getLinearIndex(uint64_t k, uint64_t i, uint64_t j);
void bitmap_setVal(uint64_t index, uint8_t set_val);
uint8_t bitmap_getVal(uint64_t index);
void bitmap_printStatistics(struct app_data* data);
void bitmap_init(struct app_data* data, int create); // decide dimensions via resolution
void bitmap_clear();


/* Default Functions */
struct node default_getNode(int k, int i, int j);
void default_setNode(int k, int i, int j, struct node val);
void default_printStatistics(struct app_data* data);
void default_init(struct app_data* data, int create); // decide dimensions via MODEL_MEM_SIZE
void default_clear(int print_progress);
void default_build(struct app_data* data, int print_progress);
