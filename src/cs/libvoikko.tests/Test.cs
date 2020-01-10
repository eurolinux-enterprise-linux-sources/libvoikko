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
 * Portions created by the Initial Developer are Copyright (C) 2011
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

using System;
using NUnit.Framework;
using System.Collections.Generic;
namespace libvoikko
{
	[TestFixture]
	public class Test
	{
		private Voikko voikko;

		[SetUp]
		public void setUp()
		{
			voikko = new Voikko("fi");
		}

		[TearDown]
		public void tearDown()
		{
			voikko.Dispose();
			// Do garbage collection after every test method. This will make errors
			// in native memory management (double frees etc.) more likely to show up.
			voikko = null;
			GC.Collect();
			GC.WaitForPendingFinalizers();
		}

		[Test]
		public void initAndTerminate()
		{
			// do nothing, just check that setUp and tearDown complete successfully
		}

		[Test]
		public void terminateCanBeCalledMultipleTimes()
		{
			voikko.Dispose();
			voikko.Dispose();
		}

		[Test]
		public void anotherObjectCanBeCreatedUsedAndDeletedInParallel()
		{
			Voikko medicalVoikko = new Voikko("fi-x-medicine");
			Assert.IsTrue(medicalVoikko.Spell("amifostiini"));
			Assert.IsFalse(voikko.Spell("amifostiini"));
			medicalVoikko.Dispose();
			Assert.IsFalse(voikko.Spell("amifostiini"));
		}

		[Test]
		public void dictionaryComparisonWorks()
		{
			VoikkoDictionary d1 = new VoikkoDictionary("fi", "a", "b");
			VoikkoDictionary d2 = new VoikkoDictionary("fi", "a", "c");
			VoikkoDictionary d3 = new VoikkoDictionary("fi", "c", "b");
			VoikkoDictionary d4 = new VoikkoDictionary("fi", "a", "b");
			VoikkoDictionary d5 = new VoikkoDictionary("sv", "a", "b");
			Assert.IsFalse(d1.Equals("kissa"));
			Assert.IsFalse("kissa".Equals(d1));
			Assert.IsFalse(d1.Equals(d2));
			Assert.IsFalse(d1.Equals(d3));
			Assert.IsFalse(d4.Equals(d5));
			Assert.IsTrue(d1.Equals(d4));
			Assert.IsTrue(d1.CompareTo(d2) < 0);
			Assert.IsTrue(d2.CompareTo(d3) < 0);
			Assert.IsTrue(d4.CompareTo(d5) < 0);
		}

		[Test]
		public void dictionaryHashCodeWorks()
		{
			VoikkoDictionary d1 = new VoikkoDictionary("fi", "a", "b");
			VoikkoDictionary d2 = new VoikkoDictionary("fi", "a", "c");
			VoikkoDictionary d3 = new VoikkoDictionary("fi", "c", "b");
			VoikkoDictionary d4 = new VoikkoDictionary("fi", "a", "b");
			VoikkoDictionary d5 = new VoikkoDictionary("sv", "a", "b");
			Assert.AreNotEqual(d1.GetHashCode(), d2.GetHashCode());
			Assert.AreNotEqual(d1.GetHashCode(), d3.GetHashCode());
			Assert.AreNotEqual(d4.GetHashCode(), d5.GetHashCode());
			Assert.AreEqual(d1.GetHashCode(), d4.GetHashCode());
		}

		[Test]
		public void listDictsWithoutPath()
		{
			List<VoikkoDictionary> dicts = Voikko.listDicts();
			Assert.IsTrue(dicts.Count > 0);
			VoikkoDictionary standard = dicts[0];
			Assert.AreEqual("standard", standard.Variant);
		}

		//[Test] TODO: should work, write test
		public void listDictsWithPathAndAttributes()
		{
			
		}

		[Test]
		public void initWithCorrectDictWorks()
		{
			voikko.Dispose();
			voikko = new Voikko("fi-x-standard");
			Assert.IsFalse(voikko.Spell("amifostiini"));
			voikko.Dispose();
			voikko = new Voikko("fi-x-medicine");
			Assert.IsTrue(voikko.Spell("amifostiini"));
		}

		[Test]
		public void initWithNonExistentDictThrowsException()
		{
			voikko.Dispose();
			try
			{
				voikko = new Voikko("fi-x-non-existent-variant");
			} catch (VoikkoException e)
			{
				Assert.AreEqual("Specified dictionary variant was not found", e.Message);
				return;
			}
			Assert.Fail("Expected exception not thrown");
		}

		[Test]
		public void initWithPathWorks()
		{
			// TODO: better test
			voikko.Dispose();
			voikko = new Voikko("fi", "/path/to/nowhere");
			Assert.IsTrue(voikko.Spell("kissa"));
		}

		[Test]
		public void spellAfterTerminateThrowsException()
		{
			voikko.Dispose();
			try
			{
				voikko.Spell("kissa");
			} catch (VoikkoException)
			{
				return;
			}
			Assert.Fail("Expected exception not thrown");
		}

		[Test]
		public void spell()
		{
			Assert.IsTrue(voikko.Spell("määrä"));
			Assert.IsFalse(voikko.Spell("määä"));
		}

		[Test]
		public void suggest()
		{
			Assert.IsTrue(voikko.Suggest("koirra").Contains("koira"));
			Assert.IsTrue(voikko.Suggest("määärä").Contains("määrä"));
			Assert.AreEqual(0, voikko.Suggest("lasjkblvankirknaslvethikertvhgn").Count);
		}

		[Test]
		public void suggestGc()
		{
			for (int i = 0; i < 10; i++)
			{
				Assert.IsTrue(voikko.Suggest("määärä").Contains("määrä"));
				GC.Collect();
				GC.WaitForPendingFinalizers();
			}
		}

		[Test]
		public void suggestReturnsArgumentIfWordIsCorrect()
		{
			List<string> suggestions = voikko.Suggest("koira");
			Assert.AreEqual(1, suggestions.Count);
			Assert.AreEqual("koira", suggestions[0]);
		}

		[Test]
		public void grammarErrorsAndExplanation()
		{
			List<GrammarError> errors = voikko.GrammarErrors("Minä olen joten kuten kaunis.");
			Assert.AreEqual(1, errors.Count);
			GrammarError error = errors[0];
			Assert.AreEqual(10, error.StartPos);
			Assert.AreEqual(11, error.ErrorLen);
			Assert.AreEqual(1, error.Suggestions.Count);
			Assert.AreEqual("jotenkuten", error.Suggestions[0]);
			int code = error.ErrorCode;
			Assert.AreEqual("Virheellinen kirjoitusasu", voikko.GrammarErrorExplanation(code, "fi"));
			Assert.AreEqual("Incorrect spelling of word(s)", voikko.GrammarErrorExplanation(code, "en"));
		}

		[Test]
		public void noGrammarErrorsInEmptyParagraph()
		{
			List<GrammarError> errors = voikko.GrammarErrors("Olen täi.\n\nOlen täi.");
			Assert.AreEqual(0, errors.Count);
		}

		[Test]
		public void grammarErrorOffsetsInMultipleParagraphs()
		{
			List<GrammarError> errors = voikko.GrammarErrors("Olen täi.\n\nOlen joten kuten.");
			Assert.AreEqual(1, errors.Count);
			Assert.AreEqual(16, errors[0].StartPos);
			Assert.AreEqual(11, errors[0].ErrorLen);
		}

		[Test]
		public void analyze()
		{
			List<Analysis> analysisList = voikko.Analyze("kansaneläkehakemus");
			Assert.AreEqual(1, analysisList.Count);
			Assert.AreEqual("=pppppp=ppppp=ppppppp", analysisList[0]["STRUCTURE"]);
		}

		[Test]
		public void tokens()
		{
			List<Token> tokens = voikko.Tokens("kissa ja koira sekä härkä");
			Assert.AreEqual(9, tokens.Count);
			Assert.AreEqual(TokenType.WORD, tokens[2].Type);
			Assert.AreEqual("ja", tokens[2].Text);
			Assert.AreEqual(TokenType.WHITESPACE, tokens[7].Type);
			Assert.AreEqual(" ", tokens[7].Text);
			Assert.AreEqual(TokenType.WORD, tokens[8].Type);
			Assert.AreEqual("härkä", tokens[8].Text);
		}

		[Test]
		public void sentences()
		{
			List<Sentence> sentences = voikko.Sentences("Kissa ei ole koira. Koira ei ole kissa.");
			Assert.AreEqual(2, sentences.Count);
			Assert.AreEqual("Kissa ei ole koira. ", sentences[0].Text);
			Assert.AreEqual(SentenceStartType.PROBABLE, sentences[0].NextStartType);
			Assert.AreEqual("Koira ei ole kissa.", sentences[1].Text);
			Assert.AreEqual(SentenceStartType.NONE, sentences[1].NextStartType);
		}

		[Test]
		public void hyphenationPattern()
		{
			Assert.AreEqual("   - ", voikko.GetHyphenationPattern("kissa"));
			Assert.AreEqual("   - ", voikko.GetHyphenationPattern("määrä"));
			Assert.AreEqual("    - =  - ", voikko.GetHyphenationPattern("kuorma-auto"));
			Assert.AreEqual("   =  ", voikko.GetHyphenationPattern("vaa'an"));
		}

		[Test]
		public void hyphenate()
		{
			Assert.AreEqual("kis-sa", voikko.Hyphenate("kissa"));
			Assert.AreEqual("mää-rä", voikko.Hyphenate("määrä"));
			Assert.AreEqual("kuor-ma-au-to", voikko.Hyphenate("kuorma-auto"));
			Assert.AreEqual("vaa-an", voikko.Hyphenate("vaa'an"));
		}

		[Test]
		public void setIgnoreDot()
		{
			voikko.IgnoreDot = false;
			Assert.IsFalse(voikko.Spell("kissa."));
			voikko.IgnoreDot = true;
			Assert.IsTrue(voikko.Spell("kissa."));
		}

		[Test]
		public void setIgnoreNumbers()
		{
			voikko.IgnoreNumbers = false;
			Assert.IsFalse(voikko.Spell("kissa2"));
			voikko.IgnoreNumbers = true;
			Assert.IsTrue(voikko.Spell("kissa2"));
		}

		[Test]
		public void setIgnoreUppercase()
		{
			voikko.IgnoreUppercase = false;
			Assert.IsFalse(voikko.Spell("KAAAA"));
			voikko.IgnoreUppercase = true;
			Assert.IsTrue(voikko.Spell("KAAAA"));
		}

		[Test]
		public void setAcceptFirstUppercase()
		{
			voikko.AcceptFirstUppercase = false;
			Assert.IsFalse(voikko.Spell("Kissa"));
			voikko.AcceptFirstUppercase = true;
			Assert.IsTrue(voikko.Spell("Kissa"));
		}

		[Test]
		public void upperCaseScandinavianLetters()
		{
			Assert.IsTrue(voikko.Spell("Äiti"));
			Assert.IsFalse(voikko.Spell("Ääiti"));
			Assert.IsTrue(voikko.Spell("š"));
			Assert.IsTrue(voikko.Spell("Š"));
		}

		[Test]
		public void acceptAllUppercase()
		{
			voikko.IgnoreUppercase = false;
			voikko.AcceptAllUppercase = false;
			Assert.IsFalse(voikko.Spell("KISSA"));
			voikko.AcceptAllUppercase = true;
			Assert.IsTrue(voikko.Spell("KISSA"));
			Assert.IsFalse(voikko.Spell("KAAAA"));
		}

		[Test]
		public void ignoreNonwords()
		{
			voikko.IgnoreNonwords = false;
			Assert.IsFalse(voikko.Spell("hatapitk@iki.fi"));
			voikko.IgnoreNonwords = true;
			Assert.IsTrue(voikko.Spell("hatapitk@iki.fi"));
			Assert.IsFalse(voikko.Spell("ashdaksd"));
		}

		[Test]
		public void acceptExtraHyphens()
		{
			voikko.AcceptExtraHyphens = false;
			Assert.IsFalse(voikko.Spell("kerros-talo"));
			voikko.AcceptExtraHyphens = true;
			Assert.IsTrue(voikko.Spell("kerros-talo"));
		}

		[Test]
		public void acceptMissingHyphens()
		{
			voikko.AcceptMissingHyphens = false;
			Assert.IsFalse(voikko.Spell("sosiaali"));
			voikko.AcceptMissingHyphens = true;
			Assert.IsTrue(voikko.Spell("sosiaali"));
		}

		[Test]
		public void setAcceptTitlesInGc()
		{
			voikko.AcceptTitlesInGc = false;
			Assert.AreEqual(1, voikko.GrammarErrors("Kissa on eläin").Count);
			voikko.AcceptTitlesInGc = true;
			Assert.AreEqual(0, voikko.GrammarErrors("Kissa on eläin").Count);
		}

		[Test]
		public void setAcceptUnfinishedParagraphsInGc()
		{
			voikko.AcceptUnfinishedParagraphsInGc = false;
			Assert.AreEqual(1, voikko.GrammarErrors("Kissa on ").Count);
			voikko.AcceptUnfinishedParagraphsInGc = true;
			Assert.AreEqual(0, voikko.GrammarErrors("Kissa on ").Count);
		}

		[Test]
		public void setAcceptBulletedListsInGc()
		{
			voikko.AcceptBulletedListsInGc = false;
			Assert.Greater(voikko.GrammarErrors("kissa").Count, 0);
			voikko.AcceptBulletedListsInGc = true;
			Assert.AreEqual(0, voikko.GrammarErrors("kissa").Count);
		}

		[Test]
		public void setNoUglyHyphenation()
		{
			voikko.NoUglyHyphenation = false;
			Assert.AreEqual("i-va", voikko.Hyphenate("iva"));
			voikko.NoUglyHyphenation = true;
			Assert.AreEqual("iva", voikko.Hyphenate("iva"));
		}

		[Test]
		public void setHyphenateUnknownWordsWorks()
		{
			voikko.HyphenateUnknownWords = false;
			Assert.AreEqual("kirjutepo", voikko.Hyphenate("kirjutepo"));
			voikko.HyphenateUnknownWords = true;
			Assert.AreEqual("kir-ju-te-po", voikko.Hyphenate("kirjutepo"));
		}

		[Test]
		public void setMinHyphenatedWordLength()
		{
			voikko.MinHyphenatedWordLength = 6;
			Assert.AreEqual("koira", voikko.Hyphenate("koira"));
			voikko.MinHyphenatedWordLength = 2;
			Assert.AreEqual("koi-ra", voikko.Hyphenate("koira"));
		}

		[Test]
		public void increaseSpellerCacheSize()
		{
			// TODO: this only tests that nothing breaks, not that cache is actually increased
			voikko.SpellerCacheSize = 3;
			Assert.IsTrue(voikko.Spell("kissa"));
		}

		[Test]
		public void disableSpellerCache()
		{
			// TODO: this only tests that nothing breaks, not that cache is actually disabled
			voikko.SpellerCacheSize = -1;
			Assert.IsTrue(voikko.Spell("kissa"));
		}

		[Test]
		public void setSuggestionStrategy()
		{
			voikko.SuggestionStrategy = SuggestionStrategy.OCR;
			Assert.IsFalse(voikko.Suggest("koari").Contains("koira"));
			Assert.IsTrue(voikko.Suggest("koir_").Contains("koira"));
			voikko.SuggestionStrategy = SuggestionStrategy.TYPO;
			Assert.IsTrue(voikko.Suggest("koari").Contains("koira"));
		}

		[Test]
		public void embeddedNullsAreNotAccepted()
		{
			Assert.IsFalse(voikko.Spell("kissa\0asdasd"));
			Assert.AreEqual(0, voikko.Suggest("kisssa\0koira").Count);
			Assert.AreEqual("kissa\0koira", voikko.Hyphenate("kissa\0koira"));
			Assert.AreEqual(0, voikko.GrammarErrors("kissa\0koira").Count);
			Assert.AreEqual(0, voikko.Analyze("kissa\0koira").Count);
		}

		[Test]
		public void nullCharMeansSingleSentence()
		{
			List<Sentence> sentences = voikko.Sentences("kissa\0koira");
			Assert.AreEqual(1, sentences.Count);
			Assert.AreEqual(SentenceStartType.NONE, sentences[0].NextStartType);
			Assert.AreEqual("kissa\0koira", sentences[0].Text);
		}

		[Test]
		public void nullCharIsUnknownToken()
		{
			{
				List<Token> tokens = voikko.Tokens("kissa\0koira");
				Assert.AreEqual(3, tokens.Count);
				Assert.AreEqual(TokenType.WORD, tokens[0].Type);
				Assert.AreEqual("kissa", tokens[0].Text);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[1].Type);
				Assert.AreEqual("\0", tokens[1].Text);
				Assert.AreEqual(TokenType.WORD, tokens[2].Type);
				Assert.AreEqual("koira", tokens[2].Text);
			}
			{
				List<Token> tokens = voikko.Tokens("kissa\0\0koira");
				Assert.AreEqual(4, tokens.Count);
				Assert.AreEqual(TokenType.WORD, tokens[0].Type);
				Assert.AreEqual("kissa", tokens[0].Text);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[1].Type);
				Assert.AreEqual("\0", tokens[1].Text);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[2].Type);
				Assert.AreEqual("\0", tokens[2].Text);
				Assert.AreEqual(TokenType.WORD, tokens[3].Type);
				Assert.AreEqual("koira", tokens[3].Text);
			}
			{
				List<Token> tokens = voikko.Tokens("kissa\0");
				Assert.AreEqual(2, tokens.Count);
				Assert.AreEqual(TokenType.WORD, tokens[0].Type);
				Assert.AreEqual("kissa", tokens[0].Text);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[1].Type);
				Assert.AreEqual("\0", tokens[1].Text);
			}
			{
				List<Token> tokens = voikko.Tokens("\0kissa");
				Assert.AreEqual(2, tokens.Count);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[0].Type);
				Assert.AreEqual("\0", tokens[0].Text);
				Assert.AreEqual(TokenType.WORD, tokens[1].Type);
				Assert.AreEqual("kissa", tokens[1].Text);
			}
			{
				List<Token> tokens = voikko.Tokens("\0");
				Assert.AreEqual(1, tokens.Count);
				Assert.AreEqual(TokenType.UNKNOWN, tokens[0].Type);
				Assert.AreEqual("\0", tokens[0].Text);
			}
			Assert.AreEqual(0, voikko.Tokens("").Count);
		}
	}
}
