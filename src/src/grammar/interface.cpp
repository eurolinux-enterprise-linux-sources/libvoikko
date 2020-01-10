/* The contents of this file are subject to the Mozilla Public License Version 
 * 1.1 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at 
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is Libvoikko: Library of natural language processing tools.
 * The Initial Developer of the Original Code is Harri Pitkänen <hatapitk@iki.fi>.
 * Portions created by the Initial Developer are Copyright (C) 2008 - 2010
 * the Initial Developer. All Rights Reserved.
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *********************************************************************************/

#include "porting.h"
#include "setup/setup.hpp"
#include "utils/StringUtils.hpp"
#include "grammar/cache.hpp"
#include <cstdlib>
#include <cstring>

typedef voikko_grammar_error VoikkoGrammarError; // for now

namespace libvoikko {

VOIKKOEXPORT VoikkoGrammarError * voikkoNextGrammarErrorUcs4(voikko_options_t * options, const wchar_t * text_ucs4,
                     size_t wtextlen, size_t startpos, int skiperrors) {
	if (text_ucs4 == 0 || wtextlen == 0) {
		return 0;
	}
	
	const voikko_grammar_error * c_error =
	    gc_error_from_cache(options, text_ucs4, startpos, skiperrors);
	if (!c_error) {
		gc_paragraph_to_cache(options, text_ucs4, wtextlen);
		c_error = gc_error_from_cache(options, text_ucs4, startpos, skiperrors);
	}
	
	if (!c_error || c_error->error_code == 0) {
		return 0;
	}
	
	// Return a deep copy of cached error
	voikko_grammar_error * e = new voikko_grammar_error();
	memcpy(e, c_error, sizeof(voikko_grammar_error));
	if (!c_error->suggestions) {
		return e;
	}
	
	int sugg_count = 0;
	for (char ** s = c_error->suggestions; *s; s++) {
		sugg_count++;
	}
	e->suggestions = new char*[sugg_count + 1];
	for (int i = 0; i < sugg_count; i++) {
		e->suggestions[i] = new char[strlen(c_error->suggestions[i]) + 1];
		strcpy(e->suggestions[i], c_error->suggestions[i]);
	}
	e->suggestions[sugg_count] = 0;
	
	return e;
}

VOIKKOEXPORT VoikkoGrammarError * voikkoNextGrammarErrorCstr(voikko_options_t * options,
		const char * text, size_t textlen, size_t startpos, int skiperrors) {
	if (text == 0 || textlen == 0) {
		return 0;
	}
	
	wchar_t * textUcs4 = utils::StringUtils::ucs4FromUtf8(text, textlen);
	if (textUcs4 == 0) {
		return 0;
	}
	
	size_t wtextlen = wcslen(textUcs4);
	VoikkoGrammarError * e = voikkoNextGrammarErrorUcs4(options, textUcs4,
	                         wtextlen, startpos, skiperrors);
	delete[] textUcs4;
	
	return e;
}

VOIKKOEXPORT int voikkoGetGrammarErrorCode(const VoikkoGrammarError * error) {
	return error->error_code;
}

VOIKKOEXPORT size_t voikkoGetGrammarErrorStartPos(const VoikkoGrammarError * error) {
	return error->startpos;
}

VOIKKOEXPORT size_t voikkoGetGrammarErrorLength(const VoikkoGrammarError * error) {
	return error->errorlen;
}

VOIKKOEXPORT const char ** voikkoGetGrammarErrorSuggestions(const VoikkoGrammarError * error) {
	// Adding const since the caller should not modify the suggestions directly.
	return const_cast<const char **>(error->suggestions);
}

VOIKKOEXPORT void voikkoFreeGrammarError(VoikkoGrammarError * error) {
	if (error) {
		if (error->suggestions) {
			for (char ** suggestion = error->suggestions; *suggestion; ++suggestion) {
				delete[] *suggestion;
			}
			delete[] error->suggestions;
		}
		delete error;
	}
}

}
