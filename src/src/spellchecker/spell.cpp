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

#include "voikko_defs.h"
#include "setup/setup.hpp"
#include "utils/utils.hpp"
#include "character/charset.hpp"
#include "spellchecker/spell.hpp"
#include "morphology/AnalyzerFactory.hpp"
#include "spellchecker/SpellUtils.hpp"
#include <cstdlib>
#include <cstring>
#include <wchar.h>
#include <wctype.h>

using namespace libvoikko::morphology;
using namespace libvoikko::spellchecker;
using namespace std;

namespace libvoikko {

static spellresult bestAnalysis(const wchar_t * word, size_t len) {
	const Analyzer * analyzer = AnalyzerFactory::getAnalyzer();
	list<Analysis *> * analyses = analyzer->analyze(word, len);
	
	if (analyses->empty()) {
		Analyzer::deleteAnalyses(analyses);
		return SPELL_FAILED;
	}
	
	spellresult best_result = SPELL_FAILED;
	list<Analysis *>::const_iterator it = analyses->begin();
	while (it != analyses->end()) {
		const wchar_t * structure = (*it)->getValue("STRUCTURE");
		spellresult result = SpellUtils::matchWordAndAnalysis(word, len, structure);
		if (best_result == SPELL_FAILED || best_result > result) {
			best_result = result;
		}
		if (best_result == SPELL_OK) {
			break;
		}
		it++;
	}
	Analyzer::deleteAnalyses(analyses);
	
	return best_result;
}

spellresult voikko_do_spell(const wchar_t * word, size_t len) {
	spellresult result_with_border = SPELL_FAILED;
	spellresult result_without_border = SPELL_FAILED;
	
	enum spellresult result = bestAnalysis(word, len);
	const wchar_t * hyphen_pos;
	if (result != SPELL_OK && len > 3) {
		hyphen_pos = wmemchr(word + 1, L'-', len - 2);
	}
	else {
		hyphen_pos = 0;
	}
	
	if (hyphen_pos) { /* Check optional hyphens */
		size_t leading_len = hyphen_pos - word;
		wchar_t * buffer = new wchar_t[len];
		wcsncpy(buffer, word, leading_len);
		wcsncpy(buffer + leading_len, hyphen_pos + 1, len - leading_len - 1);
		buffer[len - 1] = L'\0';
		
		if (voikko_options.accept_extra_hyphens && leading_len > 1 &&
		    buffer[leading_len] != L'-') {
			/* All hyphens are optional */
			/* FIXME: deep recursion */
			spellresult spres = voikko_do_spell(buffer, len - 1);
			if (spres == SPELL_OK) {
				delete[] buffer;
				return spres;
			}
		}
		
		/* Leading part ends with the same VC pair as the trailing part starts ('pop-opisto') */
		if (leading_len >= 2 && len - leading_len >= 3) {
			wint_t vctest1 = towlower(word[leading_len - 2]);
			wint_t vctest2 = towlower(word[leading_len - 1]);
			if (wcschr(VOIKKO_VOWELS, vctest1) &&
			    wcschr(VOIKKO_CONSONANTS, vctest2) &&
			    towlower(word[leading_len + 1]) == vctest1 &&
			    towlower(word[leading_len + 2]) == vctest2) {
				spellresult spres = bestAnalysis(buffer, len - 1);
				if (spres != SPELL_FAILED && (result == SPELL_FAILED || result > spres)) {
					delete[] buffer;
					return spres;
				}
			}
		}
		
		/* Ambiguous compound ('syy-silta', 'syys-ilta') */
		const Analyzer * analyzer = AnalyzerFactory::getAnalyzer();
		list<Analysis *> * analyses = analyzer->analyze(buffer);
		
		if (analyses->empty()) {
			Analyzer::deleteAnalyses(analyses);
			delete[] buffer;
			return result;
		}
		
		list<Analysis *>::const_iterator it = analyses->begin();
		while (it != analyses->end()) {
			const wchar_t * structure = (*it)->getValue("STRUCTURE");
			size_t j = 0;
			size_t i;
			for (i = 0; i < leading_len; i++) {
				while (structure[j] == L'=') {
					j++;
				}
				if (structure[j] == L'\0') {
					break;
				}
				j++;
			}
			if (i == leading_len) {
				spellresult spres = SpellUtils::matchWordAndAnalysis(buffer, len - 1, structure);
				if (structure[j] == L'=' && (result_with_border == SPELL_FAILED ||
				    result_with_border > spres)) {
					result_with_border = spres;
				}
				if (structure[j] != L'=' && (result_without_border == SPELL_FAILED ||
				    result_without_border > spres)) {
					result_without_border = spres;
				}
			}
			it++;
		}
		
		Analyzer::deleteAnalyses(analyses);
		delete[] buffer;
		if (result_with_border != SPELL_FAILED && result_without_border != SPELL_FAILED &&
		    (result == SPELL_FAILED || result > result_with_border)) {
			return result_with_border;
		}
	}
	
	return result;
}

spellresult voikko_do_spell_ignore_hyphens(const wchar_t * word, size_t len) {
	spellresult spres = voikko_do_spell(word, len);
	if (spres != SPELL_FAILED) return spres;
	
	// Hyphens were already present, so we cannot do anything more
	if (len < 2 || (word[0] == L'-' && word[len - 1] == L'-')) return SPELL_FAILED;
	
	wchar_t * buffer = new wchar_t[len + 2];
	size_t newlen = len + 1;
	if (word[0] == L'-') {
		wcsncpy(buffer, word, len);
		buffer[len] = L'-';
	}
	else {
		buffer[0] = L'-';
		wcsncpy(buffer + 1, word, len);
		if (word[len - 1] != L'-') {
			buffer[len + 1] = L'-';
			newlen++;
		}
	}
	spres = voikko_do_spell(buffer, newlen);
	delete[] buffer;
	return spres;
}

/* A small result cache:
 * word_length order elements wchars offset
 * 1           4     16       16     0
 * 2           5     32       64     16
 * 3           6     64       192    80
 * 4           7     128      512    272
 * 5           7     128      640    784
 * 6           7     128      768    1424
 * 7           7     128      896    2192
 * 8           7     128      1024   3088
 * 9           7     128      1152   4112
 * 10          7     128      1280   5264
 * total size      sizeof(wchar_t) * 6544
 */

const int VOIKKO_HASH_ORDERS[]   =  {0, 3+0,  5+0,  6+0,   7+0,   7+0,    7+0,    7+0,    7+0,    7+0,    7+0};
const int VOIKKO_CACHE_OFFSETS[] =  {0,   0, 1*16, 1*80, 1*272, 1*784, 1*1424, 1*2192, 1*3088, 1*4112, 1*5264};
const int VOIKKO_META_OFFSETS[]  =  {0,   0, 1*16, 1*48, 1*112, 1*240,  1*368,  1*496,  1*624,  1*752,  1*880};

spellresult voikko_cached_spell(const wchar_t * buffer, size_t len) {
	int sparam = voikko_options.cache_size;
	if (voikko_options.cache && len <= 10) { /* check cache */
		int hashcode = voikko_hash(buffer, len, VOIKKO_HASH_ORDERS[len] + sparam);
		int cache_offset = (VOIKKO_CACHE_OFFSETS[len] << sparam) + hashcode * static_cast<int>(len);
		int meta_offset = (VOIKKO_META_OFFSETS[len] << sparam) + hashcode;
		if (wcsncmp(voikko_options.cache + cache_offset, buffer, len) == 0) {
			/* DEBUG: printf("Cache hit: '%ls'\n", buffer);*/
			if (voikko_options.cache_meta[meta_offset] == 'i') return SPELL_CAP_FIRST;
			else return SPELL_OK;
		}
		/* not in cache */
		spellresult result;
		if (voikko_options.accept_missing_hyphens) {
			result = voikko_do_spell_ignore_hyphens(buffer, len);
		}
		else {
			result = voikko_do_spell(buffer, len);
		}
		if (result == SPELL_OK || result == SPELL_CAP_FIRST) {
			wcsncpy(voikko_options.cache + cache_offset, buffer, len);
			voikko_options.cache_meta[meta_offset] = (result == SPELL_OK) ? 'p' : 'i';
		}
		return result;
	}
	/* no cache available */
	if (voikko_options.accept_missing_hyphens) {
		return voikko_do_spell_ignore_hyphens(buffer, len);
	}
	else {
		return voikko_do_spell(buffer, len);
	}
}


VOIKKOEXPORT int voikko_spell_ucs4(int /*handle*/, const wchar_t * word) {
	size_t nchars = wcslen(word);
	int result;
	spellresult sres;
	if (nchars == 0) return VOIKKO_SPELL_OK;
	if (nchars > LIBVOIKKO_MAX_WORD_CHARS) return VOIKKO_INTERNAL_ERROR;
	ENTER_V
	
	wchar_t * nword = voikko_normalise(word, nchars);
	if (nword == 0) {
		EXIT_V
		return VOIKKO_INTERNAL_ERROR;
	}
	nchars = wcslen(nword);
	
	if (voikko_options.ignore_numbers) {
		for (size_t i = 0; i < nchars; i++) {
			if (iswdigit(nword[i])) {
				delete[] nword;
				EXIT_V
				return VOIKKO_SPELL_OK;
			}
		}
	}
	casetype caps = voikko_casetype(nword, nchars);
	if ((voikko_options.ignore_uppercase && caps == CT_ALL_UPPER) ||
	    (voikko_options.ignore_nonwords && voikko_is_nonword(nword, nchars))) {
		delete[] nword;
		EXIT_V
		return VOIKKO_SPELL_OK;
	}
	
	wchar_t * buffer = new wchar_t[nchars + 1];

	for (size_t i = 0; i < nchars; i++) {
		buffer[i] = towlower(nword[i]);
	}
	buffer[nchars] = L'\0';
	
	int dot_index;
	if (voikko_options.ignore_dot && buffer[nchars - 1] == L'.') {
		dot_index = static_cast<int>(nchars - 1);
		buffer[dot_index] = L'\0';
	}
	else {
		dot_index = -1;
	}
	
	/* Check words that require exact captialisation */
	if (caps == CT_COMPLEX || caps == CT_NO_LETTERS ||
	    (caps == CT_ALL_UPPER && !voikko_options.accept_all_uppercase)) {
		wcsncpy(buffer, nword, nchars);
		buffer[0] = towlower(buffer[0]);
		if (voikko_options.accept_missing_hyphens) {
			sres = voikko_do_spell_ignore_hyphens(buffer, nchars);
		}
		else {
			sres = voikko_do_spell(buffer, nchars);
		}
		if (sres == SPELL_OK ||
		    (sres == SPELL_CAP_FIRST && voikko_options.accept_first_uppercase && iswupper(nword[0]))) {
			result = VOIKKO_SPELL_OK;
		}
		else {
			result = VOIKKO_SPELL_FAILED;
		}
		if (result == VOIKKO_SPELL_FAILED && dot_index != -1) { /* remove dot */
			buffer[dot_index] = L'\0';
			if (voikko_options.accept_missing_hyphens) {
				sres = voikko_do_spell_ignore_hyphens(buffer, nchars);
			}
			else {
				sres = voikko_do_spell(buffer, nchars);
			}
			if (sres == SPELL_OK ||
			    (sres == SPELL_CAP_FIRST && voikko_options.accept_first_uppercase && iswupper(nword[0]))) {
				result = VOIKKO_SPELL_OK;
			}
		}
		delete[] nword;
		delete[] buffer;
		EXIT_V
		return result;
	}
	
	
	/* Check without trailing dot */
	switch (caps) {
		case CT_ALL_LOWER:
			sres = voikko_cached_spell(buffer, nchars);
			result = (sres == SPELL_OK) ? VOIKKO_SPELL_OK : VOIKKO_SPELL_FAILED;
			break;
		case CT_FIRST_UPPER:
			sres = voikko_cached_spell(buffer, nchars);
			if ((sres == SPELL_OK && voikko_options.accept_first_uppercase) || sres == SPELL_CAP_FIRST)
				result = VOIKKO_SPELL_OK;
			else result = VOIKKO_SPELL_FAILED;
			break;
		case CT_ALL_UPPER:
			assert(voikko_options.accept_all_uppercase);
			sres = voikko_cached_spell(buffer, nchars);
			result = (sres == SPELL_FAILED) ? VOIKKO_SPELL_FAILED : VOIKKO_SPELL_OK;
			break;
		default: /* should not happen */
			result = VOIKKO_INTERNAL_ERROR;
	}
	if (result == VOIKKO_SPELL_OK) {
		delete[] nword;
		delete[] buffer;
		return VOIKKO_SPELL_OK;
	}
	
	/* Check with trailing dot */
	if (dot_index != -1) {
		buffer[dot_index] = L'.';
		switch (caps) {
			case CT_ALL_LOWER:
				sres = voikko_cached_spell(buffer, nchars);
				result = (sres == SPELL_OK) ? VOIKKO_SPELL_OK : VOIKKO_SPELL_FAILED;
				break;
			case CT_FIRST_UPPER:
				sres = voikko_cached_spell(buffer, nchars);
				if ((sres == SPELL_OK && voikko_options.accept_first_uppercase) || sres == SPELL_CAP_FIRST)
					result = VOIKKO_SPELL_OK;
				else result = VOIKKO_SPELL_FAILED;
				break;
			case CT_ALL_UPPER:
				assert(voikko_options.accept_all_uppercase);
				sres = voikko_cached_spell(buffer, nchars);
				result = (sres == SPELL_FAILED) ? VOIKKO_SPELL_FAILED : VOIKKO_SPELL_OK;
			default: /* should not happen */
				result = VOIKKO_INTERNAL_ERROR;
		}
	}
	delete[] nword;
	delete[] buffer;
	return result;
}

VOIKKOEXPORT int voikko_spell_cstr(int handle, const char * word) {
	if (word == 0 || word[0] == '\0') return VOIKKO_SPELL_OK;
	size_t len = strlen(word);
	if (len > LIBVOIKKO_MAX_WORD_CHARS) return 0;
	wchar_t * word_ucs4 = voikko_cstrtoucs4(word, voikko_options.encoding, len);
	if (word_ucs4 == 0) return VOIKKO_CHARSET_CONVERSION_FAILED;
	int result = voikko_spell_ucs4(handle, word_ucs4);
	delete[] word_ucs4;
	return result;
}

}
