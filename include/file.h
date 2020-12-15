#include "globalVars.h"


int createSharedMem(void** shm_ptr, int* mem_id, unsigned long mem_size);
int destroySharedMem( void* shm_ptr, int mem_id);
int saveToFile(void* shm_ptr, unsigned long mem_size, int verbose);
int readFromFile(void* shm_ptr, unsigned long mem_size, int verbose);
