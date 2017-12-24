# Notepad 2e

*Notepad 2e* is a fork of *Notepad2* by Florian Balmer (www.flos-freeware.ch).

For information about the original project please see `Readme.txt` and `Notepad2.txt`.
This document describes *2e*-specific features (**e** for **E**xtended).

For those of you who are unfamiliar with *Notepad2*:

> ***Notepad2* is a light-weight Scintilla-based text editor for Windows.**
> It features no tabs, code folding, autocompletion, or other features
> available in IDEs or more complex editors like Notepad++.
> It's just 1 EXE file and (optionally) 1 INI file.

Some folks use it to replace the standard `Notepad.exe` of Windows. It knows about permissions and will use user-specific INI file if it cannot write INI in the EXE's directory.

## Extended Edition Changes
The `[NEW]` mark indicates a new major feature introduced by *Notepad 2e*. Items without this mark are changes (or minor features added) compared to the original *Notepad2*.

### [NEW] Current Word Highlighting
Word under cursor is highlighted in one of 3 modes:
1. One occurrence in the document. Indispensible to spot typos.
2. Two or more occurrences but all are visible on the screen.
3. Multiple occurrences with some hidden under the scrollbar.

Each mode's formatting can be customized (fill color, border style, etc.).

### [NEW] Edit Selection
Allows simultaneous editing of all occurrences of the same string as the selection. Enter the mode with **Ctrl+Tab** (all visible substrings) or **Ctrl+Backtick** (limit substrings to those on the same line). Escape cancels all changes, while Enter or any command that causes cursor to leave the selected block - commits them. #27 #18 #1

**Note:** when selection is empty, this mode affects word near the cursor, and finds other substrings case-insensitively. When selection is non-empty - case-sensitive search is performed, and word boundaries are not checked.

This mode allows easy renaming of varaibles, typo corrections and so on.

Below, with cursor within `foo` pressing **Ctrl+Tab** will enter this mode and any change you do (such as typing `bar`) will edit both `foo`s at the same time:

```
$foo = "foo";
print($foo);
```

### [NEW] Math Evaluation
In certain cases (such as in current selection - controlled by the `MathEval` setting), the file size group in the status bar is replaced with a recognized math expression's result. Left click on the group toggles the base (bin, oct, dec, hex), right click copies result (as it appears) to the clipboard.

* Selection must be shorter than 4096 symbols.
* Expression is locale-insensitive and follows English conventions so that comma is a thousands separator (and ignored) and period is a decimal part separator.
* If expression has commas but no periods and is still valid after replacing commas with periods - then the converted version is evaluated: `12,30 + ,1` = 12.40. This allows more resistance to locale-specific delimiters.
* If result has no fractional part - period and zeros are hidden from the status bar group. Fractional part is discarded if active base is any but decimal.
* The following symbols are ignored: `, $`. Whitespace is ignored unless it separates operands/operators.
* If expression contains `=` then the `=` and everything after it is ignored. Useful for checking calculations: `1+2=4` evaluates to 3.
* Special case: expression with only digits, periods and any ignored symbols (below) is treated as a series of whitespace-separated numbers, which are summed up. For example: `12,3 45.6 $78` = `123+45.6+78` = 246.6. #72

The following expression tokens are recognized:
* operators: `( ) + - * / ^` (caret works as power)
* operators: *div mod shl shr and or xor not*
* hexadecimal base prefix: `0x`
* base suffixes: `b o d h`

### [NEW] Find Next/Previous Word
Vim-like Edit > Find Next/Previous Word (**Ctrl+[Shift]+8**) commands for quick case-insensitive navigation between highlighted words. It's independent of highlight mode settings and of normal Find/Replace and doesn't affect the latter's state. #38

If there's no selection then these search for word at cursor (or for nearby next/previous word). If there's selection then these search for previously used word (not for selection!).

### [NEW] New Line Above/Below
Vim-like Edit > Lines > New Line Above/Below (**Ctrl+Alt+Enter**, **Ctrl+Shift+Enter**) commands that insert a line regardless of which column the cursor is positioned at.

If Auto Indent is enabled and the caret is already at line start/end (whitespace excluded) - indentation of the previous line is used, otherwise - of the current line. #67

### Unindent by Shift+Tab
**Shift+Tab** always unindents selected lines (or current line if empty), even if the caret isn't at line start (exactly as Edit > Block > Unindent). #128

Tab's behaviour is not changed, it still indents to the column. #61

### Find Dialog
* **[NEW]** Find (**Ctrl+F**) now has Grep/Ungrep buttons (can be quickly called with **Alt+G/R**). Find's settings including regexp mode are respected. In case of active selection these commands operate on selected lines only. Big buffers will see a progress bar in the status bar. #46 #29
* Submission buttons are disabled in regexp mode if Search String has errors.
* Checkboxes are disabled when certain combinations of flags render other flags invalid (e.g. **regexp** doesn't support **whole word**). #108
* Line feeds (`\r` and `\n`) are removed from Search/Replace Strings (can appear after pasting). #70
* Search and Replace String inputs handle **Ctrl+Backspace** hotkey (delete word before cursor). #121 #50

### [NEW] Open Previous
File > Open Previous (**Alt+G**) command lets you toggle between two most recent History files with one keystroke. #8

### [NEW] Open By Prefix
Open Dialog allows opening by prefix - so instead of typing the full file name or selecting a file with your mouse you can only type the name's beginning and hit Enter (or click Open) to open the first matching file. #19

If entered string is a wildcard (e.g. `*.txt`) then the first matching file is opened. This overrides standard Windows behaviour when the file list would be filtered instead of opening a file.

**Note:** prefix cannot match one of reserved file system names such as `NUL` and `CON` (case-insensitive).

This feature is disabled by default - enable it with `OpenDialogByPrefix` setting.

### [NEW] Open Next/Previous
File > Open Next/Previous commands allow opening files going before/after current in the currently opened file's directory (this is determined by regular name sorting, it doesn't depend on how Explorer or Open Dialog sorts files). #43

These commands have no hotkeys but they can be added as buttons to the toolbar.

### Default Save Extension
When saving, new file's extension is determined first by `DefaultExtension` setting (as in *Notepad2*) but if current file was previously opened from or saved to disk then its old extension is used as the default (even if that extension is empty). #17

When saving, if the given new file name ends on period then the file is saved without extension. Example: enter `Makefile.` to get `Makefile` on disk.

### File Dialogs
* **[NEW]** Rename To (**Alt+F6**) command acts as Save As but deletes original file on success.
* Open/Save File dialogs now start with the path of last opened file. #22
* Improved directory locking - in some cases a directory could not be removed even after opening a file in another directory due to Windows' Open Dialog operation. #100

### Insert Tag (Alt+X)
* Skips leading/trailing whitespace within the selection. This way you don't care about selection precision and can, for example, click on line number to select a full line and wrap it in `<p>` without `</p>` appearing after the line break. #30
* Remembers Opening and Closing tags until the program is closed.
* Supports `{ ( [` in addition to `<` when auto filling Closing tag. Example: `{{#if var}}` produces `{{/if}}`, `[quote]` produces `[/quote]`. #36
* Skips leading non-word symbols when determining tag name: `{{ #if }}` is the same as `{{#if}}` and produces `{{/if}}`, not `{{/}}` as in *Notepad2*.
* Supports HTML5 tags (in addition to XHTML/HTML 4.x tags from *Notepad2*). #120
* Sets **Closing tag** to **Opening tag** if the latter doesn't contain `<` and `>`. Useful when editing certain markup. #24
* In *Notepad2*, Auto Close HTML corrupts non-Latin tags in UTF-8 buffers - this is fixed. #112

### Find
* **[NEW]** Find and other commands leave certain scroll margin to preserve a customizable amount of lines (such as 33%) above and below the match. #41
* **[NEW]** The Find icon on the toolbar changes to the Stop icon whenever the search hits last result in that direction, regardless of the **Wrap around** flag.
* Find respects the **Match case** flag even with Cyrillic characters in the search string. #9

### Go To (Ctrl+G)
* Extracts first number from Line and Column instead of requiring a strictly numeric value. For example, `abc567.89` navigates to 567. #14
* [NEW] Navigation by absolute Offset in the buffer. Respects different charsets to the best effort possible. Value is normalized: #2
  1. Leading symbols that are not `0-9 A-Z a-z` are removed.
  2. If remainder consists of `0-9` then use it as a decimal offset and return.
  3. Remove possible leading `0x` and/or `h`.
  4. If remainder consists of `0-9 A-F a-f` then use it as a hexadecimal offset and return.
  5. In other cases do nothing and don't close the dialog.
* Input priority: use Offset if non-empty, else use Column+Line if Column non-empty, else use only Line.

### File > Launch
* **[NEW]** File > Launch > Shell Menu (**Ctrl+Shift+R**) command invokes Explorer's context menu for currently opened file. Current directory is set to the file's path. #12
* **[NEW]** File > Launch > Open Folder (**Ctrl+Shift+L**) command that opens Explorer's window with the current file selected. #136
* File > Launch > Command (**Ctrl+R**) retains the command string until another file is opened and sets current directory to that of the file. #26

### Line Selection Hotkeys
Due to it accidental nature, disabled triple-click and triple-**Ctrl+Space** Scintilla behaviour that previously caused selection of the entire line. Full line can still be selected with standard **Ctrl+Shift+Space** hotkey, or by clicking on the line number.

### Line Gutter
* Default gutter style was changed from `size:-2;fore:#ff0000` to `size:-1`.
* Gutter is now automatically resized if it can't fit max line number.

### PCRE Support
Replaced incomplete *Notepad2* regexp implementation with a fully-featured C++11 implementation - with `(a|b)`, backreferences `\1` (both in Search and Replace Strings) and all other features.

Additionally, old regexp didn't support UTF-8 buffers (only ASCII) - new one does.

### Enclose Selection (Alt+Q)
* Skips leading/trailing whitespace within the selection. For example, enclosindg space + `foo` + space produces space + `(foo)` + space instead of `( foo )`.
* When "before" string consists of one of these characters: `{ ( [ <` then "after" is set to the same number of `} ) ] >`.
* When "before" consists of one of the characters below then "after" is set to the same string as "before":
```
` ~ ! @ # % ^ * - _ +  = | \ / : ; " ' , . ?
```

These changes make editing Markdown and wiki sources much more pleasant: `[[foo|bar]]`, `**foo bar**`. #37

### Unwrap Commands
* **[NEW]** Edit > Block > Unwrap Brackes At Cursor (**Ctrl+Shift+3**) command compliments *Notepad2*'s **Ctrl+3-5**. This command removes brackets of type `( { [ <` around the caret (whichever bracket type appears first). Respects nesting. #39
* **[NEW]** Edit > Block > Unwrap Quotes At Cursor (**Ctrl+Shift+4**) command compliments *Notepad2*'s **Ctrl+1-2/6**. This command removes matching `" '` (and backtick) around the caret (text is scanned to the left to determine the quote type). Does not account for nesting or escaping. Multiline.

### Special Commands
HTML data:
* **[NEW]** Edit > Special > Strip HTML Tags (**Shift+Alt+X**) command removes `<tags>` inside selection, or if there's none - removes first tag before cursor. #40
* **[NEW]** Edit > Special > Escape HTML (**Ctrl+Shift+Alt+X**) command turns `< > &` into `&lt; &gt; &amp;` respectively (inside selection or everywhere in the document if selection is empty). #51

Binary data:
* **[NEW]** Edit > Special > String To Hex and Hex To String (**[Ctrl+]Alt+Shift+A**) operate on the document as a bytestream similarly to PHP's `bin2hex()` and `hex2bin()`. Hex To String ignores whitespace (#123). Output: `616263` for `abc`. #87
* **[NEW]** Edit > Special > QP Encode and QP Decode (**[Ctrl+]Alt+Shift+Q**) operate on the document as a bytestream similarly to PHP's `quoted_printable_encode()` and `quoted_printable_decode()`. Output: `ab=3Dc` for `ab=c`. See [RFC2045, Section 6.7](http://www.faqs.org/rfcs/rfc2045) (this format is typically used in vCards and MIME). #124
* **[NEW]** Edit > Special > Base64 Encode and Base64 Decode (**[Ctrl+]Alt+Shift+W**) operate on the document as a bytestream similarly to PHP's `base64_encode()` and `base64_decode()`. Output: `YWJj` for `abc`. #122
* Big buffers will see a progress bar in the status bar for the above commands.

**Warning:** if you're saving binary data (e.g. base64-encoding a binary file) make sure to disable both checkboxes in File > Line Endings > Default.

### [NEW] Ctrl+Wheel Scroll
Rolling mouse wheel while holding **Ctrl** scrolls the document by entire pages (like **Page Up/Down**) - makes it easier to navigate long scripts. #11

### Highlight Line
`HighlightLineIfWindowInactive` setting keeps current line highlighted even if the window is not focused (especially useful when using Windows' X-Mouse behaviour).

### Drop Text
When dropping an object from another application on an empty line - line break is added automatically. #63 #110

Useful when creating snippets or lists from other sources - such as from URLs; no more need to manually insert line breaks after each drop.

### Copy Add (Ctrl+E)
* Uses single line break instead of double. #28
* When called with empty selection - appends the entire line.
* In *Notepad2*, it didn't work with empty clipboard - now it works.

### Retain Position On Recode
When file is re-coded (File > Encoding menu items) caret position and selection are retained, to the best effort possible. #7

### [NEW] Go To Last Change
Go To Last Change (**Ctrl+Shift+Z**) command moves caret to the position of last Undo action. Useful when making a change, scrolling to confirm something and then navigating back to continue. #6

### Join Lines
Join Lines and Join Paragraphs (**Ctrl+[Shift+]J**) adjust selection's end so that it doesn't include trailing line break(s). #135

### Setting Commands
* **[NEW]** Replace Settings in All Instances command makes all other instances reload the INI from disk. Useful if you have dozens of *Notepad2* windows open and need to change settings in one of them and have them saved (normally when an instance quits it overwrites the INI with its own settings). With this command you can do Save Settings Now, then Replace to ensure your new settings are not overwritten. #5
* **[NEW]** Reload Settings from Disk (**Alt+F7**) command replaces all settings in current window with fresh version read from the INI file. #16
* Save Settings On Exit was extended to allow a third value: Recent Files/Search Strings. This allows you to customize Notepad 2e once, Save Settings and then not worry that temporary changes (from a particular instance) will be saved, at the same time allowing the program to save file History and Find/Replace strings. #101
* Save Settings On Exit can be also changed with a toolbar button (toggles between All... and Recent... values). #95

### [NEW] Language Indicator
Window's title reflects current keyboard language, if configured with `TitleLanguage`. For example: `Untitled - Notepad 2e [RU]`. #86

### Other Changes
* Sort Lines (**Alt+O**) and Modify Lines (**Alt+M**) operate on the entire document if selection is empty (*Notepad2* does nothing in this case). #133
* File > Encoding > UTF-8 has **Shift+F8** hotkey assigned. #21
* File > Line Endings > Unix has **Alt+F8** hotkey assigned. #44
* If large file loading stops due to memory limit, an error message is produced (*Notepad2* silently stops loading it). #126
* Upgraded Scintilla library to a more recent version (3.6.6).

### Undocumented Notepad2 Features
* Rectangular selection mode is actually supported - hold Alt while dragging your mouse to make a selection. This is particularly useful for **Column sort** in Sort Lines (**Alt+O**).
* Web search: if you set `WebTemplate1` (or `WebTemplate2`) setting in the INI to `https://google.com/search?q=%s` and then press **Ctrl+Shift+1-2** with non-empty selection - you will be navigated to that URL (`%s` replaced with a selection string, not URL-encoded).
* If Replace's dialog Replace With is `^c` - clipboard contents is used instead of this string.
* If the program is started with `/B` flag, it enters "Pasteboard mode" where new content on the clipboard is automatically added to the buffer.

### Syntax Schemes

[NEW] These syntax schemes were added:
* ASN1
* bash
* CoffeeScript
* D
* Lisp
* Lua
* Markdown
* NSIS Script
* OCaml
* Ruby
* Rust
* TeX
* YAML

CSS syntax scheme was improved:
* Added CSS 3 properties. #4
* Enabled `//`-inline comments (**Ctrl+Q**) that are used in LESS, SASS and other preprocessors. #4
* Fixed brackets of nested rules not matching in some cases (visually and when using **Ctrl+B**). #4

Lua syntax scheme was improved:
* Added single-line: `--...` and multi-line comments: `--[[...]]`. #111

## Extended Edition INI Configuration
*Notepad2* stores all of its settings in `Notepad2.ini` (or rather in `exe_base_name.ini`).
*Notepad 2e* uses the same file but puts its settings under the `[Notepad2e]` group.

These settings are all **[NEW]**.

### CSSSettings
**Type:** int, bitfield.

Extend standard CSS highlighting to support:

Bit | Type
----|-----
1 | Sassy
2 | LESS
4 | HSS

It's a bitfield so bits can be combined: **3** = Sassy + LESS.

### DebugLog
**Type:** int, bool.

Enables creation of debug log in the program's folder.

### FindWordMatchCase & FindWordWrapAround
**Type:** int, bool.

These control **Ctrl[+Shift]+8** search like normal Find dialog flags.

The **Match whole word only** flag is always enabled for those commands so it can't be customized.

### HighlightLineIfWindowInactive
**Type:** int, bool.

If **0**, current line is not highlighted if window is inactive (default *Notepad2* behaviour).

If **1**, highlighting is independent of window focus (always visible if enabled).

### MaxSearchDistance
**Type:** int, KiB.

Maximum lookahead/behind distance for word highlighting. If too large, navigation in big files will lag since it will search the buffer for twice this length (back & forward) on every position change. #53 #42

### OpenDialogByPrefix
**Type:** int, bool.

If set, Open File dialog can be submitted even if just a prefix of an existing file's name or a mask was entered.

### ScrollYCaretPolicy
**Type:** int.

Sets vertical margin for commands that can scroll the buffer, including:
* **F3, F2, Ctrl+8** and their **Shift** versions
* **Ctrl+], Ctrl+[** and their **Shift** versions
* **Page Up, Page Down** and their **Shift**, **Alt+Shift** versions

Value | Meaning
------|--------
0 | No margin (as in *Notepad2*)
1 | 33% margin
2 | 50% margin

### MathEval
**Type:** int.

Controls math expression evaluation. #88

Value | Meaning
------|--------
0 | No evaluation (as in *Notepad2*)
1 | Evaluate selection (if it's a valid expression)
2 | Evaluate selection or, if empty - entire current line

### TitleLanguage
**Type:** int.

Controls keyboard language display in window's title.

Value | Meaning
------|--------
0 | No indication (as in *Notepad2*)
1 | Always add language name as in `... [RU]`
2 | As **1** but don't add if the language is English (`EN`)

This setting is controlled by Settings > Window Title Display.

### ShellMenuType
**Type:** int, bitfield.

Controls behaviour of Shell Menu (**Ctrl+Shift+R**). For values see `uFlags` at this page:
http://msdn.microsoft.com/en-us/library/windows/desktop/bb776097(v=vs.85).aspx

### WheelScroll
**Type:** int, bool.

Enables scrolling by **Ctrl+Wheel**.

This setting is controlled by Settings > Ctrl+Wheel Scroll.

### WheelScrollInterval
**Type:** int, ms.

when using **Ctrl+Wheel**, buffer will be scrolled at most once per this interval. Necessary because Windows fires a handful of wheel scroll events per one real scroll.

### MoveCaretOnRightClick
**Type:** int, bool.

If **0** - caret is not moved and selection is not changed on right mouse button click. #54

This setting is controlled by Settings > Move Caret On Right Click.

### WordNavigationMode
**Type:** int, bool.

Controls **Ctrl+Arrow** navigation. If **1**, enables "accelerated" mode where only whitespace is considered a word boundary. #89

This setting is controlled by Settings > Ctrl+Arrow Navigation.

### SelectionType & PageSelectionType & SingleSelectionType & EditSelectionType
**Type:** int.

Type of decoration for a word that's present elsewhere in the document (`SelectionType`), on the visible page (`PageXXX`), not present at all (`SingleXXX`) or when it's in Edit Mode (`EditXXX`).

For values see:
http://www.scintilla.org/ScintillaDoc.html#Indicators

### SelectionColor & PageSelectionColor & SingleSelectionColor & EditSelectionColor
**Type:** str, BGR.

Foreground highlight color like `0xFF0000` (blue - not RGB!).

### SelectionAlpha & PageSelectionAlpha & SingleSelectionAlpha & EditSelectionAlpha
**Type:** int.

Opacity value (0-255) for foreground highlight color.

### SelectionLineAlpha &  PageSelectionLineAlpha & SingleSelectionLineAlpha & EditSelectionLineAlpha
**Type:** int.

Opacity value (0-255) for highlight outline color.

### SelectionUnder & PageSelectionUnder & SingleSelectionUnder & EditSelectionUnder
**Type:** int, bool.

Corresponds to Scintilla's `SCI_INDICSETUNDER`.
