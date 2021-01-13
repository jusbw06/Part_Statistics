#include <math.h>
#include <limits.h>

/*** Constants ***/
#define INVALID ULONG_MAX
#define CONST 4096  // reference block size
#define S_NODE sizeof(struct node)


/* Constants */
uint8_t s_dim;
uint32_t block_size;
uint32_t num_mem_blocks;
uint32_t num_hdd_blocks;
uint64_t mem_buff_size;
uint32_t current_slot;


/* 4 arrays */
void* mem_buffer;
int hdd_fd;
void* mem_hdd_map;
void* hdd_mem_map;

struct node_address{
	void* hdd_block_offset; // in bytes
	void* sub_block_offset; // in bytes
};


int mem_init(uint64_t mem_size_request, uint64_t file_size_request){

	/* Initialize Constants */
	s_dim = (uint8_t) ciel(  cbrt(((double)CONST)/S_NODE)  ); // == 6; number of nodes per dim in block
	block_size = S_NODE * pow(s_dim,3); // real block size in bytes
	num_hdd_blocks = file_size_request / block_size;
	num_mem_blocks = mem_size_request / block_size;
	mem_buff_size = num_mem_blocks*block_size;
	current_slot = 0;

	/* Initialize Arrays */
	mem_buffer = malloc(mem_buffer_size);
	hdd_fd = open64("../saves/model.sav", O_RDONLY | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	mem_hdd_map = malloc( num_mem_blocks * sizeof(void*) );
	hdd_mem_map = malloc( num_hdd_blocks * sizeof(void*) );

	/* Set to Invalid */
	for (uint32_t i = 0; i < num_mem_blocks; i++)
		mem_hdd_map[i] = INVALID;
	for (uint32_t i = 0; i < num_hdd_blocks; i++)
		hdd_mem_map[i] = INVALID;
}

void block_read(uint64_t hdd_block_offset){
	void* mem_block_curr_addr = mem_buff + current_slot*block_size; // read into memory
	pread64(hdd_fd, mem_block_curr_addr, block_size, 0 + hdd_offset);
	hdd_mem_map[hdd_offset/block_size] = mem_block_curr_addr; // <-- div hdd_offset by block size
	mem_hdd_map[current_slot] = hdd_block_offset;
}


// returns memory address of block
void* readAndReplace(uint64_t hdd_offset){ //bytes

	if (mem_hdd_map[current_slot] != INVALID){
		flush_block(current_slot); //flush
	}
	block_read(hdd_offset); // read into memory

	if (current_slot >= num_mem_blocks){
		current_slot = 0;
	}else{
		current_slot++;
	}
	return (mem_buff + current_slot*block_size);
}




// returns mem block address
(void*) retrieveBlockMemPtr(uint32_t hdd_block_offset){

	if (hdd_mem_map[hdd_block_offset/block_size] == INVALID){ // not in memory
		// check current slot --> if not free, flush --> retrieve from memory --> set array_map values
		return readAndReplace(hdd_block_offset);
	}else{
		return hdd_mem_map[hdd_block_offset/block_size];
	}
}





// i j k in nodes // check this one
struct node_address getNodeAddress(uint64_t i, uint64_t j, uint64_t k){

	struct node_address addr;

	uint64_t xsize_bytes = XDIM*S_NODE;
	uint64_t ysize_bytes = YDIM*S_NODE;
	uint64_t zsize_bytes = ZDIM*S_NODE;

	bi = i*S_NODE/s_dim;
	bj = j*S_NODE/s_dim;
	bk = k*S_NODE/s_dim;

	bir = i%s_dim;
	bjr = j%s_dim;
	bkr = k%s_dim;
	// addr_ptr
	addr.hdd_block_offset = bk * s_dim * (xsize_bytes * ysize_bytes) + bi * s_dim * (ysize_bytes) + bj * s_dim * block_size;
	//hdd_block_offset = bk * s_dim * (xsize_bytes * ysize_bytes) + bi * s_dim * (ysize_bytes) + bj * s_dim * block_size;
	addr.sub_block_offset = bkr * (s_dim * s_dim) * (S_NODE*S_NODE*S_NODE) + bir * (s_dim) * (S_NODE*S_NODE) + bjr * S_NODE;
	return addr;
}

// retrieves a node
struct node* getNode(uint64_t i, uint64_t j, uint64_t k){
	struct node_address node_addr = getNodeAddress(i, j, k);
	void* mem_ptr = retrieveBlockMemPtr(node_addr.hdd_block_offset)
	return (struct node*) (node_addr.sub_block_offset + mem_ptr);
}


void setNode(uint64_t i, uint64_t j, uint64_t k, struct node val){
	struct node_address node_addr = getNodeAddress(i, j, k);
	void* mem_ptr = retrieveBlockMemPtr(node_addr.hdd_block_offset)
	struct node* new_node = (struct node*) (node_addr.sub_block_offset + mem_ptr);
	&new_node = val;
}

// index is current memory slot
// if current memory slot is full; write contents to disk, mark respective array locations to INVALID
void flush_block(uint32_t index){
	void* mem_block_curr_addr = mem_buff + index*block_size;
	uint64_t hdd_block_offset = mem_hdd_map[index];
	pwrite64(hdd_fd, mem_block_curr_addr, block_size, 0 + hdd_block_offset);
	hdd_mem_map[hdd_block_offset/block_size] = INVALID;
	mem_hdd_map[index] = INVALID;
}

// writes all mem blocks to disk
void mem_flush_all(){
	for (int i = 0; i < num_mem_blocks; i++){
		if (mem_hdd_map[i] != INVALID){
			flush_block(i, hdd_block_offset);
		}
	}
}

