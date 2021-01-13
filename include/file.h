#include "globalVars.h"

void detachSharedMem(void* shm_ptr);
int attachToSharedMem(void** shm_ptr, int* mem_id, int shm_key);
int createSharedMem(void** shm_ptr, int* mem_id, int shm_key, unsigned long mem_size);
void destroySharedMem(void* shm_ptr, int mem_id);
int saveToFile(void* shm_ptr, unsigned long mem_size, int verbose);
int readFromFile(void* shm_ptr, unsigned long mem_size, int verbose);
