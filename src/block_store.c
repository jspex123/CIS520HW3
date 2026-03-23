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

block_store_t *block_store_create()
{
	block_store_t* block = malloc(sizeof(block_store_t));
	if (block == NULL) return NULL;

	block = memset(block, 0, sizeof(block_store_t));

	block->bitmap = bitmap_overlay(BITMAP_SIZE_BITS, block->data[BITMAP_START_BLOCK]);
	if (block->bitmap == NULL) {
		free(block);
		return NULL;
	}

	size_t bitmap_blocks = (BITMAP_SIZE_BYTES + BLOCK_SIZE_BYTES - 1) / BLOCK_SIZE_BYTES;

	for (size_t i = 0; i < bitmap_blocks; i++){
		if (!block_store_request(block, BITMAP_START_BLOCK + i)) {
			bitmap_destroy(block->bitmap);
			free(block);
			return NULL;
		}
	}

	return block;
}

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

size_t block_store_get_used_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_free_blocks(const block_store_t *const bs)
{
	UNUSED(bs);
	return 0;
}

size_t block_store_get_total_blocks()
{
	return BLOCK_STORE_NUM_BLOCKS;
}

size_t block_store_read(const block_store_t *const bs, const size_t block_id, void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

size_t block_store_write(block_store_t *const bs, const size_t block_id, const void *buffer)
{
	UNUSED(bs);
	UNUSED(block_id);
	UNUSED(buffer);
	return 0;
}

block_store_t *block_store_deserialize(const char *const filename)
{
	UNUSED(filename);
	return NULL;
}

size_t block_store_serialize(const block_store_t *const bs, const char *const filename)
{
	UNUSED(bs);
	UNUSED(filename);
	return 0;
}
