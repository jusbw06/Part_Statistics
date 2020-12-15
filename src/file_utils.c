#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<limits.h>

#include "globalVars.h"

int createSharedMem(void** shm_ptr, int* mem_id, unsigned long mem_size){
	/* Create Shared Memory */
	*mem_id = shmget(SHM_KEY, mem_size, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | IPC_CREAT );//| IPC_EXCL);
	if (*mem_id == -1){
		switch (errno){
		case EEXIST:
			fprintf(stderr,"Shared Memory Segment Already Exists\n");
			break;
		default:
			fprintf(stderr,"Error Creating Shared Memory Segment\n");
		}
		return 1;
	}
	// Attach to the segment to get a pointer to it.
	*shm_ptr = (struct shmseg*) shmat(*mem_id, NULL, 0);
	if (*shm_ptr == (void*) -1){
		fprintf(stderr, "Failed to Attach Memory Segment\n");
		return 1;
	}
	return 0;
}


int destroySharedMem(void* shm_ptr, int mem_id){

	/* Destroy Shared Memory */
	// Detach
	if (shmdt(shm_ptr) == -1) {
	  fprintf(stderr,"Failed to detach Memory Segment\n");
	  return 1;
	}
	if (shmctl(mem_id, IPC_RMID, 0) == -1) {
	  fprintf(stderr,"Failed to mark memory Segment for Destruction\n");
	  return 1;
	}
	return 0;
}

int saveToFile(void* shm_ptr, unsigned long mem_size, int verbose){

	const char* file_name = "../saves/model.sav";
	int fd;
	fd = open64(file_name, O_CREAT | O_TRUNC | O_WRONLY | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0){
		fprintf(stderr,"Error creating or opening save file\n");
		return 1;
	}

	int num_writes = 1;
	int remainder, bytes_written;
	unsigned long total_bytes_written = 0;

	while(num_writes > 0){

		num_writes = mem_size/INT_MAX;
		remainder = mem_size%INT_MAX;

		if (num_writes > 0){
			if ( (bytes_written = pwrite64(fd, shm_ptr + total_bytes_written, INT_MAX, total_bytes_written)) < 0){
				fprintf(stderr,"Error writing to file\n");
			}
			mem_size -= INT_MAX;
		}else{
			if ( (bytes_written = pwrite64(fd, shm_ptr + total_bytes_written, remainder, total_bytes_written)) < 0){
				fprintf(stderr,"Error writing to file\n");
			}
		}
		total_bytes_written += bytes_written;
		if (verbose){
			fprintf(stdout,"Wrote %d Bytes\n", bytes_written);
		}

	}

	close(fd);

	return 0;
}

int readFromFile(void* shm_ptr, unsigned long mem_size, int verbose){

	const char* file_name = "../saves/model.sav";
	int fd;
	fd = open64(file_name, O_RDONLY | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd < 0){
		fprintf(stderr,"Error opening save file\n");
		return 1;
	}
	int num_reads_left = 1;
	int remainder, bytes_read;
	unsigned long total_bytes_read = 0;


	while(num_reads_left > 0){

		num_reads_left = mem_size/INT_MAX;
		remainder = mem_size%INT_MAX;

		if (num_reads_left > 0){
			if ( (bytes_read = pread64(fd, shm_ptr + total_bytes_read, INT_MAX, total_bytes_read)) < 0){
				fprintf(stderr,"Error reading from file\n");
			}
			mem_size -= INT_MAX;
		}else{
			if ( (bytes_read = pread64(fd, shm_ptr + total_bytes_read, remainder, total_bytes_read)) < 0){
				fprintf(stderr,"Error reading from file\n");
			}
		}
		total_bytes_read += bytes_read;
		if (verbose){
			fprintf(stdout,"Read %d Bytes\n", bytes_read);
		}

	}


	close(fd);

	return 0;
}
