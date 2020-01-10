;;;; A Common Lisp interface for libvoikko
;;
;; Copyright (C) 2011-2012 Teemu Likonen <tlikonen@iki.fi>
;;
;; The contents of this file are subject to the Mozilla Public License Version
;; 1.1 (the "License"); you may not use this file except in compliance with
;; the License. You may obtain a copy of the License at
;; http://www.mozilla.org/MPL/
;;
;; Software distributed under the License is distributed on an "AS IS" basis,
;; WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
;; for the specific language governing rights and limitations under the
;; License.
;;
;; Alternatively, the contents of this file may be used under the terms of
;; either the GNU General Public License Version 2 or later (the "GPL"), or
;; the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
;; in which case the provisions of the GPL or the LGPL are applicable instead
;; of those above. If you wish to allow use of your version of this file only
;; under the terms of either the GPL or the LGPL, and not to allow others to
;; use your version of this file under the terms of the MPL, indicate your
;; decision by deleting the provisions above and replace them with the notice
;; and other provisions required by the GPL or the LGPL. If you do not delete
;; the provisions above, a recipient may use your version of this file under
;; the terms of any one of the MPL, the GPL or the LGPL.

(in-package #:voikko)

(defun spell (instance word)
  "Check the spelling of WORD. Return WORD if the spelling of correct,
otherwise return nil. INSTANCE must be an active Voikko instance, if
not, a condition of type NOT-ACTIVE-INSTANCE-ERROR is signaled."

  (error-if-not-active-instance instance)
  (let ((value (foreign-funcall "voikkoSpellCstr"
                                :pointer (address instance)
                                :string word
                                :int)))

    (ecase value
      (#.+voikko-spell-failed+ nil)
      (#.+voikko-spell-ok+ word)
      (#.+voikko-internal-error+
       (error 'internal-error :string "Internal error."))
      (#.+voikko-charset-conversion-failed+
       (error 'charset-conversion-error
              :string "Charset conversion error.")))))

(defun suggest (instance word)
  "Return spelling suggestions for WORD. Assuming that WORD is an
incorrectly spelled word the return value is a list of spelling
suggestion strings. The empty list (nil) means that no suggestions are
available. If WORD is already spelled correctly return a one-item list
containing just the original word.

INSTANCE must be an active Voikko instance, if not, a condition of type
NOT-ACTIVE-INSTANCE-ERROR is signaled."

  (error-if-not-active-instance instance)
  (let ((suggestions (foreign-funcall "voikkoSuggestCstr"
                                      :pointer (address instance)
                                      :string word
                                      :pointer)))

    (when (proper-pointer-p suggestions)
      (unwind-protect
           (loop :for i :upfrom 0
                 :for sug := (mem-aref suggestions :string i)
                 :while (stringp sug)
                 :collect sug)
        (foreign-funcall "voikkoFreeCstrArray" :pointer suggestions :void)))))

(defun hyphenate (instance word)
  "Hyphenate WORD. Two values are returned: (1) the hyphenated word as a
list of syllables (strings) and (2) the hyphenation pattern string
returned by C function voikkoHyphenateCstr(). The length of the
hyphenation pattern is the same as WORD's and the pattern consists of
the following characters:

    #\\Space = no hyphenation at this character,
    #\\-     = hyphenation point (character at this position
              is preserved in the hyphenated form),
    #\\=     = hyphenation point (character at this position
              is replaced by the hyphen.)

INSTANCE must be an active Voikko instance, if not, a condition of type
NOT-ACTIVE-INSTANCE-ERROR is signaled."

  (error-if-not-active-instance instance)
  (let ((hyphenation (foreign-funcall "voikkoHyphenateCstr"
                                      :pointer (address instance)
                                      :string word
                                      :pointer)))

    (if (proper-pointer-p hyphenation)
        (unwind-protect
             (loop :with pattern := (foreign-string-to-lisp hyphenation)
                   :with hyph
                   :with start := 0
                   :for pos :upfrom 0
                   :for h :across pattern

                   :if (char= h #\=) :do
                   (when (< start pos)
                     (push (subseq word start pos) hyph))
                   (setf start (1+ pos))

                   :else :if (char= h #\-) :do
                   (when (< start pos)
                     (push (subseq word start pos) hyph))
                   (setf start pos)

                   :finally
                   (when (< start (length word))
                     (push (subseq word start) hyph))
                   (return (values (nreverse hyph) pattern)))

          (foreign-funcall "voikkoFreeCstr" :pointer hyphenation :void))

        (error 'hyphenation-error :string "Hyphenation error."))))

(defun split-word (instance width word)
  "Split WORD in two and return the parts as a cons cell. The word is
split at correct hyphenation points only. The WIDTH argument defines the
maximun number of characters allowed in the first part.

INSTANCE must be an active Voikko instance, if not, a condition of type
NOT-ACTIVE-INSTANCE-ERROR is signaled."

  (cond ((>= width (length word))
         (cons word ""))
        ((<= width 0)
         (cons "" word))
        (t
         (loop :with hyph := (nth-value 1 (hyphenate instance word))
               :with end1 := 0
               :with start2 := 0

               :for i :from 0 :below (length word)
               :for h := (aref hyph i)
               :while (<= i width)

               :if (char= #\- h) :do (setf end1 i start2 i)
               :else :if (char= #\= h) :do (setf end1 i start2 (1+ i))

               :finally (return (cons (subseq word 0 end1)
                                      (subseq word start2)))))))
