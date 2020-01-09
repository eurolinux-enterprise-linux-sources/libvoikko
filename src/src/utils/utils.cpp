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
#include "utils/utils.hpp"
#include "setup/setup.hpp"
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#ifndef HAVE_ICONV
  #ifdef WIN32
    #include <windows.h>
  #endif
#endif

namespace libvoikko {

wchar_t * voikko_cstrtoucs4(const char * word, const char * encoding, size_t len) {
#ifdef HAVE_ICONV
	iconv_t cd;
	bool using_temporary_converter = false;
	size_t conv_bytes;
	size_t outbytesleft = len * sizeof(wchar_t);
	wchar_t * ucs4_buffer = new wchar_t[len + 1];
	if (ucs4_buffer == 0) return 0;
	ucs4_buffer[len] = L'\0';
	char * outptr = (char *) ucs4_buffer;
	ICONV_CONST char * inptr = (ICONV_CONST char *) word;
	size_t inbytesleft;
	
	inbytesleft = len;
	
	if (strcmp(encoding, "UTF-8") == 0) cd = voikko_options.iconv_utf8_ucs4;
	else if (strcmp(encoding, voikko_options.encoding) == 0) cd = voikko_options.iconv_ext_ucs4;
	else {
		cd = iconv_open(INTERNAL_CHARSET, encoding);
		if (cd == (iconv_t) -1) {
			delete[] ucs4_buffer;
			return 0;
		}
		using_temporary_converter = true;
	}
	
	LOG(("voikko_cstrtoucs4\n"));
	LOG(("inbytesleft = %d\n", (int) inbytesleft));
	LOG(("outbytesleft = %d\n", (int) outbytesleft));
	LOG(("inptr = '%s'\n", inptr));
	iconv(cd, 0, &inbytesleft, &outptr, &outbytesleft);
	conv_bytes = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
	LOG(("conv_bytes = %d\n", (int) conv_bytes));
	LOG(("inbytesleft = %d\n", (int) inbytesleft));
	LOG(("outbytesleft = %d\n", (int) outbytesleft));
	LOG(("inptr = '%s'\n", inptr));
	if (conv_bytes == (size_t) -1) {
		if (using_temporary_converter) iconv_close(cd);
		delete[] ucs4_buffer;
		return 0;
	}
	
	if (using_temporary_converter) iconv_close(cd);
	*((wchar_t *) outptr) = L'\0'; /* terminate the output buffer */
	LOG(("ucs4_buffer = '%ls'\n", ucs4_buffer));
	return ucs4_buffer;
#elif defined(WIN32)
	int cp;
	if (strcmp(encoding, "UTF-8") == 0) cp = CP_UTF8;
	else if (strcmp(encoding, "CP850") == 0) cp = 850;
	else return 0;
	// On Windows we actually use UTF-16 data internally, so two units may be needed to
	// represent a single character.
	size_t buflen = (len * 2 + 1);
	wchar_t * ucs4_buffer = new wchar_t[buflen];
	// Illegal characters are ignored, because MB_ERR_INVALID_CHARS is not supported
	// before WIN2K_SP4.
	int result = MultiByteToWideChar(cp, 0, word, len, ucs4_buffer, buflen - 1);
	if (result == 0) {
		delete[] ucs4_buffer;
		return 0;
	}
	else {
		ucs4_buffer[result] = L'\0';
		return ucs4_buffer;
	}
#else
  #error A charset conversion implementation is mandatory.
#endif
}

char * voikko_ucs4tocstr(const wchar_t * word, const char * encoding, size_t len) {
	const size_t OUTPUT_BUFFER_BYTES = LIBVOIKKO_MAX_WORD_CHARS * 6;
#ifdef HAVE_ICONV
	iconv_t cd;
	bool using_temporary_converter = false;
	size_t conv_bytes;
	char * utf8_buffer = new char[OUTPUT_BUFFER_BYTES + 1];
	char * outptr = utf8_buffer;
	ICONV_CONST char * inptr = (ICONV_CONST char *) word;
	size_t inbytesleft;
	size_t outbytesleft = OUTPUT_BUFFER_BYTES;
	utf8_buffer[OUTPUT_BUFFER_BYTES] = '\0';
	
	if (len == 0) len = wcslen(word);
	inbytesleft = len * sizeof(wchar_t);
	
	if (strcmp(encoding, "UTF-8") == 0) cd = voikko_options.iconv_ucs4_utf8;
	else if (strcmp(encoding, voikko_options.encoding) == 0) cd = voikko_options.iconv_ucs4_ext;
	else {
		cd = iconv_open(encoding, INTERNAL_CHARSET);
		if (cd == (iconv_t) -1) {
			delete[] utf8_buffer;
			return 0;
		}
		using_temporary_converter = true;
	}
	
	LOG(("voikko_ucs4tocstr\n"));
	LOG(("inbytesleft = %d\n", (int) inbytesleft));
	LOG(("outbytesleft = %d\n", (int) outbytesleft));
	LOG(("inptr = '%s'\n", inptr));
	iconv(cd, 0, &inbytesleft, &outptr, &outbytesleft);
	conv_bytes = iconv(cd, &inptr, &inbytesleft, &outptr, &outbytesleft);
	LOG(("conv_bytes = %d\n", (int) conv_bytes));
	LOG(("inbytesleft = %d\n", (int) inbytesleft));
	LOG(("outbytesleft = %d\n", (int) outbytesleft));
	LOG(("inptr = '%s'\n", inptr));
	if (conv_bytes == (size_t) -1 || inbytesleft > 0) {
		if (using_temporary_converter) iconv_close(cd);
		delete[] utf8_buffer;
		return 0;
	}
	if (using_temporary_converter) iconv_close(cd);
	*outptr = '\0'; /* terminate the output buffer */
	LOG(("utf8_buffer = '%s'\n", utf8_buffer));
	return utf8_buffer;
#elif defined(WIN32)
	int cp;
  	if (strcmp(encoding, "UTF-8") == 0) cp = CP_UTF8;
	else if (strcmp(encoding, "CP850") == 0) cp = 850;
	else return 0;
  	size_t buflen = OUTPUT_BUFFER_BYTES + 1;
  	char * utf8_buffer = new char[buflen];
  	if (utf8_buffer == 0) return 0;
  	int result = WideCharToMultiByte(cp, 0, word, len ? (int) len : -1, utf8_buffer, buflen - 1, 0, 0);
  	if (result == 0) {
		delete[] utf8_buffer;
		return 0;
	}
	else {
		utf8_buffer[result] = '\0';
		return utf8_buffer;
	}
#else
  #error A charset conversion implementation is mandatory.
#endif
}

int voikko_hash(const wchar_t * word, size_t len, int order) {
	int hash = 0;
	for (size_t counter = 0; counter < len; counter++) {
		hash = (hash * 37 + word[counter]) % (1 << order);
	}
	return hash;
}

enum casetype voikko_casetype(const wchar_t * word, size_t nchars) {
	bool first_uc = false;
	bool rest_lc = true;
	bool all_uc = true;
	bool no_letters = true;
	if (nchars == 0) return CT_NO_LETTERS;
	if (iswupper(word[0])) {
		first_uc = true;
		no_letters = false;
	}
	if (iswlower(word[0])) {
		all_uc = false;
		no_letters = false;
	}
	for (size_t i = 1; i < nchars; i++) {
		if (iswupper(word[i])) {
			no_letters = false;
			rest_lc = false;
		}
		if (iswlower(word[i])) {
			all_uc = false;
			no_letters = false;
		}
	}
	if (no_letters) return CT_NO_LETTERS;
	if (all_uc) return CT_ALL_UPPER;
	if (!rest_lc) return CT_COMPLEX;
	if (first_uc) return CT_FIRST_UPPER;
	else return CT_ALL_LOWER;
}

void voikko_set_case(enum casetype charcase, wchar_t * word, size_t nchars) {
	if (nchars == 0) return;
	switch (charcase) {
		case CT_NO_LETTERS:
		case CT_COMPLEX:
			return; /* Do nothing */
		case CT_ALL_LOWER:
			for (size_t i = 0; i < nchars; i++) {
				word[i] = towlower(word[i]);
			}
			return;
		case CT_ALL_UPPER:
			for (size_t i = 0; i < nchars; i++) {
				word[i] = towupper(word[i]);
			}
			return;
		case CT_FIRST_UPPER:
			word[0] = towupper(word[0]);
			for (size_t i = 1; i < nchars; i++) {
				word[i] = towlower(word[i]);
			}
			return;
	}
}

bool voikko_is_nonword(const wchar_t * word, size_t nchars) {
	// If X is a character (possibly other than '.'), then the following
	// patterns (URLs and email addresses) will be considered non-words:
	//   X*//X*.X+
	//   X*@X+.X+
	//   www.X+.X+
	
	if (nchars < 4) return false;
	
	const wchar_t * i = wmemchr(word, L'/', nchars - 3);
	if (i && i[1] == L'/' && wmemchr(i + 1, L'.', nchars - (i - word) - 2)) {
		return true;
	}
	
	i = wmemchr(word, L'@', nchars - 3);
	if (i && i[1] != L'.' && wmemchr(i + 1, L'.', nchars - (i - word) - 2)) {
		return true;
	}
	
	if (nchars < 7) {
		return false;
	}
	if ((wcsncmp(L"www.", word, 4) == 0) &&
	    word[4] != L'.' &&
	    wmemchr(word + 5, L'.', nchars - 5)) {
		return true;
	}
	
	return false;
}

}
