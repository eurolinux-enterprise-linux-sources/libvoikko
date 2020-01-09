/* Copyright (C) 1995 Bjoern Beutel. */

/* Description. =============================================================*/

/* This module contains structures and functions for the run-time lexicon. */

/* Includes. ================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <setjmp.h>
#include "morphology/malaga/basic.hpp"
#include "morphology/malaga/pools.hpp"
#include "morphology/malaga/values.hpp"
#include "morphology/malaga/tries.hpp"
#include "morphology/malaga/files.hpp"
#include "morphology/malaga/malaga_files.hpp"
#include "morphology/malaga/lexicon.hpp"

namespace libvoikko { namespace morphology { namespace malaga {

/* Variables. ===============================================================*/

static void *lexicon_data; /* Address of lexicon file mapped into memory. */
static int_t lexicon_length; /* Length of *LEXICON_DATA. */

static struct /* The run time lexicon. */
{ 
  int_t *trie; /* A trie with indices to FEAT_LISTS. */
  int_t trie_size;
  int_t trie_root; /* Index of root node in TRIE. */

  int_t *feat_lists; /* Lists of feature structures, stored in VALUES.
		      * (The last index I of each list is negative, 
		      * real index is abs(I) - 1.) */
  int_t feat_lists_size;

  cell_t *values; /* Feature structures of lexicon entries. */
  int_t values_size;
} lexicon;

static int_t feat_list_index, trie_node;
static string_t prefix_end;

/* Functions. ===============================================================*/

void 
search_for_prefix( string_t string )
/* Search lexicon for prefixes of STRING in increasing length. 
 * The results are obtained by calling "get_next_prefix". */
{
  trie_node = lexicon.trie_root;
  prefix_end = string;
  feat_list_index = -1;
}

/*---------------------------------------------------------------------------*/

bool 
get_next_prefix( string_t *string_p, value_t *feat )
/* Get the next lexicon entry that is a prefix of STRING. 
 * Return false iff no more entries exist.
 * If another entry exists, set *STRING_P to the remainder of STRING
 * and *FEAT to the feature structure assigned to the lexicon entry.
 * STRING must have been set by "search_for_prefix". */
{
  int_t feat_index;
  
  if (feat_list_index == -1) 
    lookup_trie( lexicon.trie, &trie_node, &prefix_end, &feat_list_index );
  if (feat_list_index == -1) 
    return false;
  feat_index = lexicon.feat_lists[ feat_list_index++ ];
  if (feat_index < 0) 
  { 
    feat_list_index = -1;
    feat_index = - feat_index - 1;
  }
  *string_p = prefix_end;
  *feat = lexicon.values + feat_index;
  return true;
}

/*---------------------------------------------------------------------------*/

void 
init_lexicon( string_t file_name )
/* Initialise this module. Read lexicon from file FILE_NAME. */
{ 
  lexicon_header_t *header; /* Lexicon file header. */

  /* Map the lexicon file into memory. */
  map_file( file_name, &lexicon_data, &lexicon_length );

  /* Check lexicon header. */
  header = (lexicon_header_t *) lexicon_data;
  check_header( &header->common_header, file_name, LEXICON_FILE,
                MIN_LEXICON_CODE_VERSION, LEXICON_CODE_VERSION );
  
  /* Init trie. */
  lexicon.trie_size = header->trie_size;
  lexicon.trie = (int_t *) (header + 1);
  lexicon.trie_root = header->trie_root;

  /* Init feature structure lists. */
  lexicon.feat_lists_size = header->feat_lists_size;
  lexicon.feat_lists = (int_t *) (lexicon.trie + lexicon.trie_size);

  /* Init values. */
  lexicon.values_size = header->values_size;
  lexicon.values = (cell_t *) (lexicon.feat_lists + lexicon.feat_lists_size);
}

/*---------------------------------------------------------------------------*/

void 
terminate_lexicon( void )
/* Terminate this module. */
{ 
  unmap_file( &lexicon_data, lexicon_length ); 
}

}}}
