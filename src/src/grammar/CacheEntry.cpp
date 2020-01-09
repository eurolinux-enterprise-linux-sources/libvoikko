/* Libvoikko: Library of Finnish language tools
 * Copyright (C) 2009 Harri Pitkänen <hatapitk@iki.fi>
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

#include "grammar/CacheEntry.hpp"
#include "grammar/error.hpp"

namespace libvoikko { namespace grammar {

CacheEntry::CacheEntry(size_t suggestionCount) :
	nextError(0) {
	init_grammar_error(&(this->error));
	if (suggestionCount > 0) {
		error.suggestions = new char*[suggestionCount + 1];
		for (size_t i = 0; i < suggestionCount + 1; i++) {
			error.suggestions[i] = 0;
		}
	}
}

} }
