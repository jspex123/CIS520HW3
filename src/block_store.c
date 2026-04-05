#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bitmap.h"
#include "block_store.h"
// include more if you need


// You might find this handy. I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

struct block_store {
    uint8_t  data[BLOCK_STORE_NUM_BLOCKS][BLOCK_SIZE_BYTES];
    bitmap_t *bitmap;
};

/**
 * Creates the block_store_t object, allocating its memory
 * @returns the address for the block_store_t object, if fails NULL
 */
block_store_t *block_store_create()
{
	// allocate memory for the block_store
	block_store_t* block = malloc(sizeof(block_store_t));
	// NULL check to make sure malloc was successful
	if (block == NULL) return NULL;

	// set all the memory in the block to 0s
	block = memset(block, 0, sizeof(block_store_t));

	// create the bitmap and assign it to the block_store object
	block->bitmap = bitmap_overlay(BITMAP_SIZE_BITS, block->data[BITMAP_START_BLOCK]);
	// NULL check for the bitmap creation
	if (block->bitmap == NULL) {
		free(block);
		return NULL;
	}

	size_t bitmap_blocks = (BITMAP_SIZE_BYTES + BLOCK_SIZE_BYTES - 1) / BLOCK_SIZE_BYTES;

	// allocates memory for the blocks, if any fail, free the block_store
	for (size_t i = 0; i < bitmap_blocks; i++){
		if (!block_store_request(block, BITMAP_START_BLOCK + i)) {
			bitmap_destroy(block->bitmap);
			free(block);
			return NULL;
		}
	}

	return block;
}

/**
 * Destroys a block store and frees all associated memory.
 * @param the Pointer to the block store to destroy
 */
void block_store_destroy(block_store_t *const bs)
{
	//null checks first, if exists -> destroy bitmap via prebuilt function and free struct memory
	if(bs){
		if(bs->bitmap){
			bitmap_destroy(bs->bitmap);
		}
		free(bs);
	}
}

/**
 * Allocates the first available free block in the block store
 * @param bs Pointer to the block store
 * @returns The allocated block id if successful, SIZE_MAX on failure
 */
size_t block_store_allocate(block_store_t *const bs)
{
	if(bs == NULL) return SIZE_MAX;

	size_t block_id = bitmap_ffz(bs->bitmap);

	while(block_id != SIZE_MAX){

		//skip the reserved blocks
		if(block_id >= BITMAP_START_BLOCK && block_id < BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS){

			//mark used
			bitmap_set(bs->bitmap, block_id);
			block_id = bitmap_ffz(bs->bitmap);
			continue;
		}

		//allocation
		bitmap_set(bs->bitmap, block_id);

		if(bitmap_test(bs->bitmap, block_id)){
			return block_id;
		}
		else{
			return SIZE_MAX;
		}
	}
	
	return SIZE_MAX;
}

/**
 * Attempts to mark a specific block as allocated in the bitmap
 * @param bs Pointer to the block store
 * @param block_id The block identifier to to mark as allocated
 * @returns true if the block was successfully marked, false otherwise
 */
bool block_store_request(block_store_t *const bs, const size_t block_id)
{
	//null check and validate that block id is within range
	if(bs == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS) {
		return false;
	}

	//check if the requested block is already marked as used in the bitmap
	if(bitmap_test(bs->bitmap, block_id)) {
		return false;
	}

	//mark the block as used in the bitmap
	bitmap_set(bs->bitmap, block_id);
	
	return bitmap_test(bs->bitmap, block_id);
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
	if (bs == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS) {
        return;
    }

	size_t bitmap_start = BITMAP_START_BLOCK;
    size_t bitmap_end   = BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS;
    if (block_id >= bitmap_start && block_id < bitmap_end) {
        return;
    }

    bitmap_reset(bs->bitmap, block_id);
}

/**
 * Gets the total number of used blocks in the block_store_t object
 * @param bs Pointer to the block store
 * @returns the number of used blocks, on fail SIZE_MAX
 */
size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	// NULL check for valid block store object
	if (bs == NULL) return SIZE_MAX;

	// returns the total number of used bits in the bitmap
	return bitmap_total_set(bs->bitmap);
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	if (bs == NULL) {
        return SIZE_MAX;
    }

    return BLOCK_STORE_NUM_BLOCKS - bitmap_total_set(bs->bitmap);
}

/**
 * Gets total number of blocks in block store
 * @returns The total number of blocks in the block_store_t object
 */
size_t block_store_get_total_blocks()
{
	// returning the number of blocks
	return BLOCK_STORE_NUM_BLOCKS;
}

/**
 * Reads data from a block into the provided buffer
 * @param bs Pointer to the block store
 * @param block_id The block identifier to read from
 * @param buffer Buffer to store the read data
 * @returns Number of bytes read on success, 0 on failure
 */
size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	//NULL check and verifies that block_id is in valid range
	if(bs == NULL || buffer == NULL || block_id >= BLOCK_STORE_NUM_BLOCKS){
		return 0;
	}

	//verifies that the requested block is currently allocated
	if(!bitmap_test(bs->bitmap, block_id)){
		return 0;
	}
	
	//copy one full block from block store into buffer
	memcpy(buffer, bs->data[block_id], BLOCK_SIZE_BYTES);
	return BLOCK_SIZE_BYTES;
}

/**
 * Writes data from a buffer to the designated block
 * @param bs Pointer to the block store
 * @param block_id The block identifier to read from
 * @param buffer Buffer to write the data 
 * @returns The number of bytes written, 0 on fail
 */
size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	// NULL checks and verifying block_id is less than the number of block and that block is allocated
	if (bs == NULL || buffer == NULL) return 0;
	if (block_id >= BLOCK_STORE_NUM_BLOCKS) return 0;
	if (!bitmap_test(bs->bitmap, block_id)) return 0;

	// validates that the block attempting to be written to is not part of the bitmap-reserved range
	if (block_id >= BITMAP_START_BLOCK &&
		 block_id < BITMAP_START_BLOCK + BITMAP_NUM_BLOCKS) return 0;

	// write the buffer to the block
	memcpy(bs->data[block_id], buffer, BLOCK_SIZE_BYTES);
	return BLOCK_SIZE_BYTES;
}

/**
 * Deserializes a block store from a file
 * @param filename Path to the file containing the serialized block store
 * @returns Pointer to newly created block_store_t, NULL on failure
 */
block_store_t *block_store_deserialize(const char *const filename)
{
	//NULL check
	if (filename == NULL) return NULL;

	//opening filestream and follow-up NULL check
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL) return NULL;

	//creation of new block_store_t and handling of allocation failure
	block_store_t *bs = block_store_create();
	if (bs == NULL){
		fclose(fp);
		return NULL;
	}

	//writing to the block data region and handles smaller file than expected
	size_t bytes_read = fread(bs->data, 1, BLOCK_STORE_NUM_BYTES, fp);
	if(bytes_read != BLOCK_STORE_NUM_BYTES){
		fclose(fp);
		block_store_destroy(bs);
		return NULL;
	}

	fclose(fp);
	return bs;

}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}
