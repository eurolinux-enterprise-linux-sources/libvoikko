/* Voikkospell: Testing tool for libvoikko
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

#include "../voikko.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <wchar.h>
#include <iostream>
#include <string>

using namespace std;

static const int MAX_WORD_LENGTH = 5000;

static bool autotest = false;
static bool suggest = false;
static bool morphology = false;
static int one_line_output = false;
static char word_separator = ' ';
static bool space = false;  /* Set to true if you want to output suggestions that has spaces in them. */

void printMorphology(int handle, const wchar_t * word) {
	voikko_mor_analysis ** analysisList =
	    voikko_analyze_word_ucs4(handle, word);
	for (voikko_mor_analysis ** analysis = analysisList;
	     *analysis; analysis++) {
		const char ** keys = voikko_mor_analysis_keys(*analysis);
		for (const char ** key = keys; *key; key++) {
			wcout << L"A(" << word << L"):";
			wcout << (analysis - analysisList) + 1 << L":";
			wcout << *key << "=";
			wcout << voikko_mor_analysis_value_ucs4(*analysis, *key);
			wcout << endl;
		}
	}
	voikko_free_mor_analysis(analysisList);
}

void check_word(int handle, const wchar_t * word) {
	int result = voikko_spell_ucs4(handle, word);
	if (result == VOIKKO_CHARSET_CONVERSION_FAILED) {
		cerr << "E: charset conversion failed" << endl;
		return;
	}
	if (result == VOIKKO_INTERNAL_ERROR) {
		cerr << "E: internal error" << endl;
		return;
	}
	if (autotest) {
		if (result) {
			wcout << L"C" << endl;
		}
		else {
			wcout << L"W" << endl;
		}
		fflush(0);
	}
	else if (one_line_output) {
		wcout << word;
		if (!result) {
			wchar_t ** suggestions = voikko_suggest_ucs4(handle, word);
			if (suggestions) {
				for (int i = 0; suggestions[i] != 0; i++) {
					if (space || wcschr(suggestions[i], L' ')) {
						wcout << word_separator;
						wcout << suggestions[i];
					}
				}
				voikko_free_suggest_ucs4(suggestions);
			}
		}
		wcout << endl;
	}
	else {
		if (result) {
			wcout << L"C: " << word << endl;
		}
		else {
			wcout << L"W: " << word << endl;
		}
	}
	if (morphology && result) {
		printMorphology(handle, word);
	}
	if (!one_line_output && suggest && !result) {
		wchar_t ** suggestions = voikko_suggest_ucs4(handle, word);
		if (suggestions) {
			for (int i = 0; suggestions[i] != 0; i++) {
				wcout << L"S: " << suggestions[i] << endl;
			}
			voikko_free_suggest_ucs4(suggestions);
		}
	}
}

/**
 * Print a list of available dictionaries to stdout.
 * @return status code to be returned when the program exits.
 */
int list_dicts(const char * path) {
	voikko_dict ** dicts = voikko_list_dicts(path);
	if (!dicts) {
		cerr << "E: Failed to list available dictionaries." << endl;
		return 1;
	}
	for (voikko_dict ** i = dicts; *i; i++) {
		cout << voikko_dict_variant(*i);
		cout << ": ";
		cout << voikko_dict_description(*i);
		cout << endl;
	}
	voikko_free_dicts(dicts);
	return 0;
}


int main(int argc, char ** argv) {
	const char * path = 0;
	const char * variant = "";
	int handle;
	int cache_size;
	
	cache_size = 0;
	bool list_dicts_requested = false;
	for (int i = 1; i < argc; i++) {
		string args(argv[i]);
		if (args.find("-c") == 0) {
			cache_size = atoi(argv[i] + 2);
		}
		else if (args == "-p" && i + 1 < argc) {
			path = argv[++i];
		}
		else if (args == "-d" && i + 1 < argc) {
			variant = argv[++i];
		}
		else if (args == "-l") {
			list_dicts_requested = true;
		}
	}
	
	if (list_dicts_requested) {
		return list_dicts(path);
	}
	
	const char * voikko_error = (const char *) voikko_init_with_path(&handle, variant, cache_size, path);
	if (voikko_error) {
		cerr << "E: Initialization of Voikko failed: " << voikko_error << endl;
		return 1;
	}
	
	for (int i = 1; i < argc; i++) {
		string args(argv[i]);
		if (args == "-t") {
			autotest = true;
		}
		else if (args == "ignore_dot=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_DOT, 1);
		else if (args == "ignore_dot=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_DOT, 0);
		else if (args == "ignore_numbers=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_NUMBERS, 1);
		else if (args == "ignore_numbers=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_NUMBERS, 0);
		else if (args == "ignore_nonwords=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_NONWORDS, 1);
		else if (args == "ignore_nonwords=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_IGNORE_NONWORDS, 0);
		else if (args == "accept_first_uppercase=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_FIRST_UPPERCASE, 1);
		else if (args == "accept_first_uppercase=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_FIRST_UPPERCASE, 0);
		else if (args == "accept_extra_hyphens=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_EXTRA_HYPHENS, 1);
		else if (args == "accept_extra_hyphens=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_EXTRA_HYPHENS, 0);
		else if (args == "accept_missing_hyphens=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_MISSING_HYPHENS, 1);
		else if (args == "accept_missing_hyphens=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_ACCEPT_MISSING_HYPHENS, 0);
		else if (args == "ocr_suggestions=1")
			voikko_set_bool_option(handle, VOIKKO_OPT_OCR_SUGGESTIONS, 1);
		else if (args == "ocr_suggestions=0")
			voikko_set_bool_option(handle, VOIKKO_OPT_OCR_SUGGESTIONS, 0);
		else if (args.find("-x") == 0) {
			one_line_output = true;
			if (args.size() == 3) {
				word_separator = argv[i][2];
			}
			space = (word_separator != ' ');
		}
		else if (args == "-s") {
			suggest = true;
		}
		else if (args == "-m") {
			morphology = true;
		}
		else if (args.find("-c") == 0) {
			continue;
		}
		else if (args == "-p" || args == "-d") {
			i++;
			continue;
		}
		else {
			cerr << "Unknown option " << args << endl;
			return 1;
		}
	}
	
	wchar_t * line = new wchar_t[MAX_WORD_LENGTH + 1];
	if (!line) {
		cerr << "E: Out of memory" << endl;
	}
	
	// Use stdout in wide character mode and stderr in narrow character mode.
	setlocale(LC_ALL, "");
	fwide(stdout, 1);
	fwide(stderr, -1);
	while (fgetws(line, MAX_WORD_LENGTH, stdin)) {
		size_t lineLen = wcslen(line);
		if (lineLen == 0) {
			continue;
		}
		if (line[lineLen - 1] == L'\n') {
			line[lineLen - 1] = L'\0';
			lineLen--;
		}
		if (lineLen > LIBVOIKKO_MAX_WORD_CHARS) {
			cerr << "E: Too long word" << endl;
			continue;
		}
		check_word(handle, line);
	}
	int error = ferror(stdin);
	if (error) {
		cerr << "E: Error while reading from stdin" << endl;
	}
	delete[] line;
	
	voikko_terminate(handle);
	return 0;
}
