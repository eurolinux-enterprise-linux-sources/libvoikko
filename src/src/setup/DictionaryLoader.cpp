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
 * Portions created by the Initial Developer are Copyright (C) 2008 - 2013
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

#include "setup/DictionaryLoader.hpp"
#include "setup/LanguageTag.hpp"
#include "porting.h"
#include <string>
#include <fstream>
#include <cstdlib>
#ifdef WIN32
# include <windows.h>
#else
# include <pwd.h>
# include <dirent.h>
# include <unistd.h>
#endif

#ifdef HAVE_HFST
#include <ZHfstOspeller.h>
#include <ospell.h>
#include <ol-exceptions.h>
#endif

#define VOIKKO_DICTIONARY_FILE "voikko-fi_FI.pro"
#define MALAGA_DICTIONARY_VERSION "2"
#define HFST_DICTIONARY_VERSION "3"
#define MALAGA_DICTIONARY_VERSION_KEY "info: Voikko-Dictionary-Format: " MALAGA_DICTIONARY_VERSION
#ifdef WIN32
# define VOIKKO_KEY                   "SOFTWARE\\Voikko"
# define VOIKKO_VALUE_DICTIONARY_PATH "DictionaryPath"
# define BUFFER_LENGTH 200
#endif

using namespace std;

namespace libvoikko { namespace setup {

static void tagToCanonicalForm(string & languageTag) {
	for (size_t i = 0; i < languageTag.size(); ++i) {
		char current = languageTag.at(i);
		if (current >= 65 && current <= 90) {
			languageTag[i] = current + 32;
		}
	}
}

static LanguageTag parseFromBCP47(const string & language) {
	// TODO: this parsing algorithm is incomplete
	LanguageTag tag;
	if (language.size() < 2) {
		return tag;
	}
	
	string canonicalLanguage = language;
	tagToCanonicalForm(canonicalLanguage);
	
	size_t languageEnd = canonicalLanguage.find("-");
	if (languageEnd == string::npos) {
		tag.setLanguage(canonicalLanguage);
	} else {
		if (languageEnd < 2) {
			// Invalid tag "-..." or "f-..."
			return tag;
		}
		tag.setLanguage(canonicalLanguage.substr(0, languageEnd));
	}
	
	size_t privateUseStart = canonicalLanguage.find("-x-");
	if (privateUseStart != string::npos) {
		string privateUse = canonicalLanguage.substr(privateUseStart + 3);
		for (size_t hyphenPos = privateUse.find("-"); hyphenPos != string::npos;
		            hyphenPos = privateUse.find("-")) {
			privateUse.erase(hyphenPos, 1);
		}
		tag.setPrivateUse(privateUse);
	}
	
	return tag;
}

/**
 * Returns true if the given variant map contains a default dictionary for given language.
 */
static bool hasDefaultForLanguage(map<string, Dictionary> & variants, const string & language) {
	for (map<string, Dictionary>::iterator i = variants.begin(); i != variants.end(); ++i) {
		if (i->second.getLanguage().getLanguage() == language && i->second.isDefault()) {
			return true;
		}
	}
	return false;
}

list<Dictionary> DictionaryLoader::findAllAvailable() {
	return findAllAvailable(string());
}


list<Dictionary> DictionaryLoader::findAllAvailable(const std::string & path) {
	list<string> locations = getDefaultLocations();
	if (!path.empty()) {
		locations.push_front(path);
	}
	
	map<string, Dictionary> dictMap;
	for (list<string>::iterator i = locations.begin(); i != locations.end(); ++i) {
		addVariantsFromPath(*i, dictMap);
	}
	
	list<Dictionary> dicts;
	for (map< string, Dictionary >::iterator i = dictMap.begin(); i != dictMap.end(); ++i) {
		if (i->second.isDefault()) {
			dicts.push_front(i->second);
		}
		else if (i->first.rfind("-x-standard") == i->first.size() - 11 &&
		         !hasDefaultForLanguage(dictMap, i->second.getLanguage().getLanguage())) {
			dicts.push_front(i->second);
		}
		else {
			dicts.push_back(i->second);
		}
	}
	return dicts;
}

Dictionary DictionaryLoader::load(const string & language) throw(DictionaryException) {
	return load(language, string());
}

static bool isMatchingLanguage(const LanguageTag & requested, const LanguageTag & available) {
	if (requested.getLanguage() != available.getLanguage()) {
		return false;
	}
	if (!requested.getPrivateUse().empty() && requested.getPrivateUse() != available.getPrivateUse()) {
		return false;
	}
	return true;
}

Dictionary DictionaryLoader::load(const string & language, const string & path)
		throw(DictionaryException) {
	LanguageTag requestedTag = parseFromBCP47(language);
	
	list<Dictionary> dicts = findAllAvailable(path);
	if (dicts.empty()) {
		throw DictionaryException("No valid dictionaries were found");
	}
	
	const string privateUse = requestedTag.getPrivateUse();
	if (privateUse.empty() || privateUse == "default" || privateUse == "fi_FI") {
		// Use dictionary specified by environment variable VOIKKO_DICTIONARY_PATH
		// XXX: Not actually thread safe but will most probably work
		char * dict_from_env = getenv("VOIKKO_DICTIONARY");
		if (dict_from_env) {
			requestedTag.setPrivateUse(string(dict_from_env));
		}
	}
	
	for (list<Dictionary>::iterator i = dicts.begin(); i != dicts.end(); ++i) {
		LanguageTag availableTag = (*i).getLanguage();
		if (isMatchingLanguage(requestedTag, availableTag)) {
			return *i;
		}
	}
	throw DictionaryException("Specified dictionary variant was not found");
}

static list<string> getListOfSubentries(const string & mainPath) {
	list<string> results;
#ifdef WIN32
	string searchPattern(mainPath);
	searchPattern.append("\\*");
	WIN32_FIND_DATA dirData;
	HANDLE handle = FindFirstFile(searchPattern.c_str(), &dirData);
	if (handle == INVALID_HANDLE_VALUE) {
		return results;
	}
	do {
		string dirName(dirData.cFileName);
#else
	DIR * dp = opendir(mainPath.c_str());
	if (!dp) {
		return results;
	}
	while (dirent * dirp = readdir(dp)) {
		string dirName(dirp->d_name);
#endif
		results.push_back(dirName);
#ifdef WIN32
	} while (FindNextFile(handle, &dirData) != 0);
	FindClose(handle);
#else
	}
	closedir(dp);
#endif
	return results;
}

void DictionaryLoader::addVariantsFromPathMalaga(const string & path, map<string, Dictionary> & variants) {
	string mainPath(path);
	mainPath.append("/");
	mainPath.append(MALAGA_DICTIONARY_VERSION);
	list<string> subDirectories = getListOfSubentries(mainPath);
	for (list<string>::iterator i = subDirectories.begin(); i != subDirectories.end(); ++i) {
		string dirName = *i;
		if (dirName.find("mor-") != 0) {
			continue;
		}
		string variantName = dirName.substr(4);
		if (variantName.empty()) {
			continue;
		}
		string fullDirName(mainPath);
		fullDirName.append("/");
		fullDirName.append(dirName);
		Dictionary dict = dictionaryFromPath(fullDirName);
		if (variantName == "default" && !hasDefaultForLanguage(variants, dict.getLanguage().getLanguage())) {
			dict.setDefault(true);
		}
		if (dict.isValid()) {
			if (variants.find(dict.getLanguage().toBcp47()) == variants.end()) {
				variants[dict.getLanguage().toBcp47()] = dict;
			}
			else if (dict.isDefault()) {
				variants[dict.getLanguage().toBcp47()].setDefault(true);
			}
		}
	}
}

void DictionaryLoader::addVariantsFromPathHfst(const string & path, map<string, Dictionary> & variants) {
#ifdef HAVE_HFST
	string mainPath(path);
	mainPath.append("/");
	mainPath.append(HFST_DICTIONARY_VERSION);
	list<string> subDirectories = getListOfSubentries(mainPath);
	for (list<string>::iterator i = subDirectories.begin(); i != subDirectories.end(); ++i) {
		string dirName = *i;
		if (dirName.find(".zhfst") + 6 == dirName.length()) {
			string fullPath = mainPath + "/" + dirName;
			string morBackend = "null";
			string spellBackend = "hfst";
			string suggestionBackend = "hfst";
			// TODO implement null hyphenator
			string hyphenatorBackend = "AnalyzerToFinnishHyphenatorAdapter(currentAnalyzer)";
			
			hfst_ol::ZHfstOspeller * speller = new hfst_ol::ZHfstOspeller();
			try {
				speller->read_zhfst(fullPath.c_str());
			}
			catch (hfst_ol::ZHfstZipReadingError& zhzre) {
				delete speller;
				continue; // broken dictionary
			}
			const hfst_ol::ZHfstOspellerXmlMetadata spellerMetadata = speller->get_metadata();
			
			LanguageTag language;
			language.setBcp47(spellerMetadata.info_.locale_);
			map<string, string> languageVersions = spellerMetadata.info_.title_;
			string description = languageVersions[spellerMetadata.info_.locale_];
			delete speller;
			Dictionary dict = Dictionary(fullPath, morBackend, spellBackend, suggestionBackend,
			                        hyphenatorBackend, language, description);
			// TODO copy-paste from above
			if (language.getPrivateUse() == "default" && !hasDefaultForLanguage(variants, dict.getLanguage().getLanguage())) {
				dict.setDefault(true);
			}
			if (dict.isValid()) {
				if (variants.find(dict.getLanguage().toBcp47()) == variants.end()) {
					variants[dict.getLanguage().toBcp47()] = dict;
				}
				else if (dict.isDefault()) {
					variants[dict.getLanguage().toBcp47()].setDefault(true);
				}
			}
		}
	}
#else
	(void)path;
	(void)variants;
#endif
}

void DictionaryLoader::addVariantsFromPath(const string & path, map<string, Dictionary> & variants) {
	addVariantsFromPathHfst(path, variants);
	addVariantsFromPathMalaga(path, variants);
}

Dictionary DictionaryLoader::dictionaryFromPath(const string & path) {
	string fileName(path);
	fileName.append("/");
	fileName.append(VOIKKO_DICTIONARY_FILE);
	
	string line;
	ifstream file(fileName.c_str(), ifstream::in);
	if (file.good()) {
		getline(file, line);
	}
	if (line.compare(MALAGA_DICTIONARY_VERSION_KEY) != 0) {
		// Not a valid dictionary for this version of libvoikko
		file.close();
		return Dictionary();
	}
	
	LanguageTag language;
	language.setLanguage("fi");
	string description;
	string morBackend = "malaga";
	string spellBackend = "FinnishSpellerTweaksWrapper(AnalyzerToSpellerAdapter(currentAnalyzer),currentAnalyzer)";
	string suggestionBackend = "FinnishSuggestionStrategy(currentAnalyzer)";
	string hyphenatorBackend = "AnalyzerToFinnishHyphenatorAdapter(currentAnalyzer)";
	while (file.good()) {
		getline(file, line);
		if (line.find("info: Language-Code: ") == 0) {
			language.setLanguage(string(line.substr(21)));
		}
		else if (line.find("info: Language-Variant: ") == 0) {
			string variant = line.substr(24);
			tagToCanonicalForm(variant);
			language.setPrivateUse(variant);
		}
		else if (line.find("info: Description: ") == 0) {
			description = line.substr(19);
		}
		else if (line.find("info: Morphology-Backend: ") == 0) {
			morBackend = line.substr(26);
		}
		else if (line.find("info: Speller-Backend: ") == 0) {
			spellBackend = line.substr(23);
		}
		else if (line.find("info: Suggestion-Backend: ") == 0) {
			suggestionBackend = line.substr(26);
		}
		else if (line.find("info: Hyphenator-Backend: ") == 0) {
			hyphenatorBackend = line.substr(26);
		}
	}
	file.close();
	return Dictionary(path, morBackend, spellBackend, suggestionBackend,
	                  hyphenatorBackend, language, description);
}

list<string> DictionaryLoader::getDefaultLocations() {
	list<string> locations;
	
	#ifndef DISABLE_EXTDICTS
	/* Path specified by environment variable VOIKKO_DICTIONARY_PATH */
	// XXX: Not actually thread safe but will most probably work
	char * path_from_env = getenv("VOIKKO_DICTIONARY_PATH");
	if (path_from_env) {
		locations.push_back(string(path_from_env));
	}

	#ifdef HAVE_GETPWUID_R
	/* $HOME/.voikko/VOIKKO_DICTIONARY_FILE */
	passwd * pwdResult;
	char * pwdBuf = new char[10000];
	if (pwdBuf) {
		passwd pwd;
		getpwuid_r(getuid(), &pwd, pwdBuf, 10000, &pwdResult);
		if (pwdResult && pwd.pw_dir) {
			string pwdPath(pwd.pw_dir);
			pwdPath.append("/.voikko");
			locations.push_back(pwdPath);
		}
		delete[] pwdBuf;
	}
	
	/* /etc on the same systems where getpwuid_r is available */
	locations.push_back("/etc/voikko");
	#endif
	
	#ifdef WIN32
	/* User default dictionary from Windows registry */
	HKEY hKey;
	LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER, VOIKKO_KEY,
						0, KEY_QUERY_VALUE, &hKey);
	char buffer[BUFFER_LENGTH];
	DWORD dwBufLen = BUFFER_LENGTH;
	if (ERROR_SUCCESS == lRet) {
		lRet = RegQueryValueEx(hKey, VOIKKO_VALUE_DICTIONARY_PATH, NULL, NULL,
		                       (LPBYTE)buffer, &dwBufLen);
		RegCloseKey(hKey);
		if ((ERROR_SUCCESS == lRet)) {
			string dirName(buffer);
			locations.push_back(dirName);
		}
	}
	
	/* System default dictionary from Windows registry */
	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, VOIKKO_KEY,
		                    0, KEY_QUERY_VALUE, &hKey);
	dwBufLen = BUFFER_LENGTH;
	if (ERROR_SUCCESS == lRet) {
		lRet = RegQueryValueEx(hKey, VOIKKO_VALUE_DICTIONARY_PATH, NULL, NULL,
		                       (LPBYTE)buffer, &dwBufLen);
		RegCloseKey(hKey);
		if ((ERROR_SUCCESS == lRet)) {
			string dirName(buffer);
			locations.push_back(dirName);
		}
	}
	#endif // WIN32
	
	#ifdef DICTIONARY_PATH
	/* Directory specified on compile time */
	locations.push_back(DICTIONARY_PATH);
	#endif
	
	#endif
	
	return locations;
}

} }
