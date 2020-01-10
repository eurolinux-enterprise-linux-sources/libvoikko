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
 * Portions created by the Initial Developer are Copyright (C) 2006 - 2010
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

/**
 * This file contains the deprecated public API of libvoikko. The
 * API is still supported although some options no longer have any
 * effect. If you are developing an application that does not need
 * to work with older versions of libvoikko it is recommended to
 * avoid using any of the symbols or macros defined here.
 *
 * You can test that your program is not using any of deprecated API
 * by compiling it with -DVOIKKO_NO_DEPRECATED_API. If the program
 * compiles, you are fine. Do not set this as default compiler option
 * though, since your application may fail to build again after an
 * otherwise backwards compatible upgrade of libvoikko.
 *
 * Everything here will be removed permanently when an API incompatible
 * version of libvoikko is released.
 *
 * NOTICE ABOUT THREAD SAFETY:
 * 
 * It is unsafe to call voikko_init, voikko_init_with_path and
 * voikko_terminate from multiple threads. If you need to do that, all
 * calls to these three functions should be protected with a single mutex.
 * Rather than use this deprecated API please switch to the new API where
 * this particular thread safety issue does not exist.
 */

/**
 * This is an integer option constant. The option no longer has any effect
 * and similar functionality is not provided by the new API. No applications
 * are known to have ever used this.
 */
#define VOIKKO_INTERSECT_COMPOUND_LEVEL 5

/**
 * This is an string option constant. The option no longer has any effect
 * and similar functionality is not provided by the new API. The values for
 * this option were never documented and the option was declared deprecated
 * for a long time before actual implementation was removed.
 */
#define VOIKKO_OPT_ENCODING 2

/**
 * Maximum number of analyses for a word. Not strictly enforced anymore.
 * Backends should still limit the number of analyses if necessary to
 * avoid combinatorial explosion.
 */
#define LIBVOIKKO_MAX_ANALYSIS_COUNT 31

/**
 * See the notice about thread safety at the top of this file.
 * @param langcode the language code. The following values can be used:
 *        - "", "default" or "fi_FI": Use the default dictionary. The default
 *          dictionary can be assumed to be present in a complete installation of
 *          libvoikko.
 *        - any other string: Use the specified dictionary variant. Usually there
 *          is at least the "standard" variant, but this is not guaranteed. If the
 *          specified dictionary does not exist, an error message is returned.
 *        - NULL: Reserved for future use. Currently leads to undefined behavior.
 * For info on other parameters see documentation for voikkoInit and
 * VOIKKO_SPELLER_CACHE_SIZE.
 */
const char * voikko_init(int * handle, const char * langcode, int cache_size);

/**
 * See the notice about thread safety at the top of this file.
 * @param langcode the language code. The following values can be used:
 *        - "", "default" or "fi_FI": Use the default dictionary. The default
 *          dictionary can be assumed to be present in a complete installation of
 *          libvoikko.
 *        - any other string: Use the specified dictionary variant. Usually there
 *          is at least the "standard" variant, but this is not guaranteed. If the
 *          specified dictionary does not exist, an error message is returned.
 *        - NULL: Reserved for future use. Currently leads to undefined behavior.
 * For info on other parameters see documentation for voikkoInit and
 * VOIKKO_SPELLER_CACHE_SIZE.
 */
const char * voikko_init_with_path(int * handle, const char * langcode,
                                   int cache_size, const char * path);

/**
 * See the notice about thread safety at the top of this file.
 * See voikkoTerminate
 */
int voikko_terminate(int handle);

/**
 * See voikkoSetBooleanOption
 */
int voikko_set_bool_option(int handle, int option, int value);

/**
 * See voikkoSetIntegerOption
 */
int voikko_set_int_option(int handle, int option, int value);

/**
 * Sets a string option. Only used for deprecated VOIKKO_OPT_ENCODING, therefore
 * no replacement has been provided yet.
 * @param handle voikko instance
 * @param option option name
 * @param value option value
 * @return true if option was succesfully set, otherwise false
 */
int voikko_set_string_option(int handle, int option, const char * value);

/**
 * See voikkoSpellCstr
 */
int voikko_spell_cstr(int handle, const char * word);

/**
 * See voikkoSpellUcs4
 */
int voikko_spell_ucs4(int handle, const wchar_t * word);

/**
 * See voikkoSuggestCstr
 */
char ** voikko_suggest_cstr(int handle, const char * word);

/**
 * See voikkoSuggestUcs4
 */
wchar_t ** voikko_suggest_ucs4(int handle, const wchar_t * word);

/**
 * See voikkoHyphenateCstr
 */
char * voikko_hyphenate_cstr(int handle, const char * word);

/**
 * See voikkoHyphenateUcs4
 */
char * voikko_hyphenate_ucs4(int handle, const wchar_t * word);

/**
 * See voikkoFreeCstrArray
 */
void voikko_free_suggest_cstr(char ** suggest_result);

/**
 * See voikkoFreeCstr
 */
void voikko_free_hyphenate(char * hyphenate_result);

/**
 * See voikkoNextTokenUcs4
 */
enum voikko_token_type voikko_next_token_ucs4(int handle, const wchar_t * text,
                       size_t textlen, size_t * tokenlen);

/**
 * See voikkoNextTokenCstr
 */
enum voikko_token_type voikko_next_token_cstr(int handle, const char * text,
                       size_t textlen, size_t * tokenlen);

/**
 * See voikkoNextSentenceStartUcs4
 */
enum voikko_sentence_type voikko_next_sentence_start_ucs4(int handle,
                          const wchar_t * text, size_t textlen, size_t * sentencelen);

/**
 * See voikkoNextSentenceStartCstr
 */
enum voikko_sentence_type voikko_next_sentence_start_cstr(int handle,
                          const char * text, size_t textlen, size_t * sentencelen);

/**
 * See voikkoNextGrammarErrorUcs4
 */
voikko_grammar_error voikko_next_grammar_error_ucs4(int handle, const wchar_t * text,
                     size_t textlen, size_t startpos, int skiperrors);

/**
 * See voikkoNextGrammarErrorCstr
 */
voikko_grammar_error voikko_next_grammar_error_cstr(int handle, const char * text,
                     size_t textlen, size_t startpos, int skiperrors);

/**
 * See voikkoAnalyzeWordUcs4
 */
struct voikko_mor_analysis ** voikko_analyze_word_ucs4(
                              int handle, const wchar_t * word);

/**
 * See voikkoAnalyzeWordCstr
 */
struct voikko_mor_analysis ** voikko_analyze_word_cstr(
                              int handle, const char * word);
