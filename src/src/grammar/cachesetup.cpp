/* Libvoikko: Finnish spellchecker and hyphenator library
 * Copyright (C) 2008 Harri Pitkänen <hatapitk@iki.fi>
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

#include "grammar/cachesetup.hpp"
#include "grammar/error.hpp"
#include "setup/setup.hpp"
#include "utils/StringUtils.hpp"
#include <cstring>
#include <cstdlib>

using namespace libvoikko::grammar;

namespace libvoikko {

voikko_gc_cache::voikko_gc_cache() :
	paragraph(0),
	firstError(0) {
}

void voikko_gc_cache::clear() {
	paragraph = 0;
	firstError = 0;
}

void gc_clear_cache(int /*handle*/) {
	if (voikko_options.gc_cache.paragraph) {
		delete[] voikko_options.gc_cache.paragraph;
	}
	CacheEntry * entry = voikko_options.gc_cache.firstError;
	while (entry) {
		CacheEntry * next = entry->nextError;
		utils::StringUtils::deleteCStringArray(entry->error.suggestions);
		delete entry;
		entry = next;
	}
	voikko_options.gc_cache.clear();
}

}
