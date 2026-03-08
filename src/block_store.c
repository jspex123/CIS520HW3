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
	UNUSED(bs);
	return 0;
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
	return true;
}

void block_store_release(block_store_t *const bs, const size_t block_id)
{
	UNUSED(bs);
	UNUSED(block_id);
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
	return 0;
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
