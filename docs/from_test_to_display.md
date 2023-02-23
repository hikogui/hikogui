From Text to Screen
===================

HikoGUI uses `hi::text` as its native text-type for use in the text-widget, text-field, text-shaper and labels.
A `hi::text` is a string of `hi::character` (`std::basic_string<hi::character>`).

Each `hi::character` contains the following information:
 - The full grapheme-cluster as specified by: Unicode Annex #29
 - The language and region of the text-run, the script can be derived from the current and surrounding characters.
 - The phrasing of the text-run, simular:
   [https://developer.mozilla.org/en-US/docs/Web/HTML/Content_categories#phrasing_content](HTML Content Categories / phrasing).
 - The text-theme to use, the actual style is selected from the other information in the character.

This information is needed for:
 - Determine text-style, fonts and text-runs.
 - Properly render the text-run by the open-type font-file, this requires the language and script.
 - Spell and grammar check requires the language and region of a text-run.
 - Text-to-speech requires the language and region to select a voice.


Conversion to `hi::text`
------------------------

