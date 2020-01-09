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

#include "spellchecker/suggestion/SuggestionGeneratorDeletion.hpp"
#include "spellchecker/suggestion/SuggestionGeneratorCaseChange.hpp"
#include <wchar.h>
#include <wctype.h>

namespace libvoikko { namespace spellchecker { namespace suggestion {

void SuggestionGeneratorDeletion::generate(SuggestionStatus * s) const {
	wchar_t * buffer = new wchar_t[s->getWordLength()];
	for (size_t i = 0; i < s->getWordLength() && !s->shouldAbort(); i++) {
		if (i == 0 || towlower(s->getWord()[i]) != towlower(s->getWord()[i-1])) {
			wcsncpy(buffer, s->getWord(), i);
			wcsncpy(buffer + i, s->getWord() + (i + 1), s->getWordLength() - i);
			SuggestionGeneratorCaseChange::suggestForBuffer(s, buffer,
			    s->getWordLength() - 1);
		}
	}
	delete[] buffer;
}

}}}
