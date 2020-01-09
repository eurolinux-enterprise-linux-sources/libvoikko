/* Libvoikko: Finnish spellchecker and hyphenator library
 * Copyright (C) 2006 - 2009 Harri Pitkänen <hatapitk@iki.fi>
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

#ifndef VOIKKO_SETUP_SETUP_H
#define VOIKKO_SETUP_SETUP_H

#include "grammar/cachesetup.hpp"
#ifdef HAVE_ICONV
  #include <iconv.h>
#endif
#include <wchar.h>

namespace libvoikko {

/** Suggestion types */
enum suggtype {ST_STD, ST_OCR};

typedef struct {
	int ignore_dot;
	int ignore_numbers;
	int ignore_uppercase;
	int ignore_nonwords;
	int accept_first_uppercase;
	int accept_all_uppercase;
	int no_ugly_hyphenation;
	int accept_extra_hyphens;
	int accept_missing_hyphens;
	int intersect_compound_level;
	size_t min_hyphenated_word_length;
	int accept_titles_in_gc;
	int accept_unfinished_paragraphs_in_gc;
	int accept_bulleted_lists_in_gc;
	int hyphenate_unknown_words;
	const char * encoding;
	#ifdef HAVE_ICONV
	iconv_t iconv_ucs4_utf8;
	iconv_t iconv_utf8_ucs4;
	iconv_t iconv_ucs4_ext;
	iconv_t iconv_ext_ucs4;
	#endif
	wchar_t * cache;
	char * cache_meta;
	int cache_size;
	enum suggtype suggestion_type;
	voikko_gc_cache gc_cache;
} voikko_options_t;

extern voikko_options_t voikko_options;

extern int voikko_handle_count;

}

#endif
