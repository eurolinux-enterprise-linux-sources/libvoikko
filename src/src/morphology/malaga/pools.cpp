/* Copyright (C) 1995 Bjoern Beutel.
 *               2009 Harri Pitkänen <hatapitk@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *********************************************************************************/

/* Description. =============================================================*/

/* This module defines a new data type, "pool_t", for growing vectors of items
 * of an arbitrary type.

 * A pool is implemented as a linked list of chunks each of which is of fixed
 * size once allocated. Individual subvectors of items may be allocated at
 * once, and it is guaranteed that these subvectors are contiguous (like
 * arrays) .*/

/* Includes. ================================================================*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "morphology/malaga/basic.hpp"
#include "morphology/malaga/files.hpp"
#include "morphology/malaga/pools.hpp"

namespace libvoikko { namespace morphology { namespace malaga {

/* Constants. ===============================================================*/

enum {MIN_CHUNK_SIZE = 400};
/* The minimum size of the data area in a chunk. */

/* Macros. ==================================================================*/

#define CHUNK_DATA(chunk_p) ((u_byte_t *) ((chunk_p) + 1))
/* Use this macro to access the data in a chunk. */

/* Types. ===================================================================*/

typedef struct /* A block of memory that is part of a pool. */
{
  // cppcheck-suppress unusedStructMember 
  list_node_t *next; /* Needed for padding */
  int_t chunk_size; /* The maximum number of items in this chunk. */
  int_t item_count; /* The actual number of items in this chunk. */
  /* For 64-bit pointers, we are 8-byte aligned here.
   * For 32-bit pointers, we are 4-byte aligned here. */
  /* Items follow here. */
} chunk_t;

/* The pool implementation type.
 * (Use "new_pool" before you use any other functions on a pool variable.) */
struct pool
{ 
  int_t item_size; /* Size of the items that are stored in this pool. */
  int_t item_count; /* The overall number of items stored in this pool. */
  int_t chunk_size; /* The size of new chunks. */
  list_t chunk_list; /* The chunks of this pool. */
};

/* Functions. ===============================================================*/

pool_t 
new_pool( int_t item_size )
/* Create a new pool that records items of size ITEM_SIZE. */
{
  pool_t pool;

  pool = (pool_t) new_mem( sizeof( struct pool ) );
  pool->item_size = item_size;
  pool->item_count = 0;
  pool->chunk_size = MIN_CHUNK_SIZE / item_size;
  clear_list( &pool->chunk_list );
  return pool;
}

/*---------------------------------------------------------------------------*/

void 
clear_pool( pool_t pool )
/* Clear POOL. Do not free any memory used by the pool. */
{
  /* Free all chunks of this pool. */
  while (pool->chunk_list.first != NULL) 
    free_first_node( &pool->chunk_list );
  pool->item_count = 0;
}

/*---------------------------------------------------------------------------*/

void *
get_pool_space( pool_t pool, int_t item_count, int_t *index )
/* Get space for ITEM_COUNT contiguous items in POOL.
 * Return its address as the function's result.
 * Return its index in *INDEX, if INDEX != NULL. */
{
  void *new_p; /* Pointer to the pool space. */
  chunk_t *chunk; /* Address of a chunk pointer. */

  /* Check if there is enough space in the last chunk. */
  chunk = (chunk_t *) pool->chunk_list.last;
  if (chunk == NULL || chunk->item_count + item_count > chunk->chunk_size) 
  { 
    if (pool->chunk_size < item_count) 
      pool->chunk_size = item_count;
    chunk = (chunk_t *) new_node( &pool->chunk_list,
		      sizeof( chunk_t ) + pool->item_size * pool->chunk_size,
		      LIST_END );
    chunk->chunk_size = pool->chunk_size;
    chunk->item_count = 0;
  }

  /* Remember address and index of current position in pool. */
  new_p = (void *) (CHUNK_DATA( chunk ) + pool->item_size * chunk->item_count);
  if (index != NULL) 
    *index = pool->item_count;

  /* Adjust indices. */
  chunk->item_count += item_count;
  pool->item_count += item_count;
 
  return new_p;
}

/*---------------------------------------------------------------------------*/

void 
free_pool( pool_t *pool )
/* Free all memory used by *POOL. */
{
  if (*pool == NULL) 
    return;
  clear_pool( *pool );
  free_mem( pool );
}

}}}
