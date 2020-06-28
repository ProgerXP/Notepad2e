# Notepad 2e

![License: 3-clause BSD](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)
![Status: x86 build](https://proger.me/notepad2e/status/1.svg)
![Status: tests](https://proger.me/notepad2e/status/2.svg)

*Notepad 2e* is a fork of *Notepad2* by Florian Balmer (www.flos-freeware.ch), version 4.2.25.

For information about the original project please see `Readme.txt` and `Notepad2.txt`.
This document describes *2e*-specific features (**e** for **E**xtended).

For those of you who are unfamiliar with *Notepad2*:

> ***Notepad2* is a light-weight Scintilla-based text editor for Windows.** <br>
> It offers no tabs, code folding, autocompletion, or other features <br>
> available in IDEs or more complex editors like Notepad++. <br>
> It's just 1 portable EXE file and (optionally) 1 INI file.

Some folks use it to replace the standard `Notepad.exe` of Windows.

It's also part of [TortoiseGit](https://tortoisegit.org).

**License:** *Notepad2* uses 3-clause BSD license. *Notepad 2e* follows the same license.

**Thanks to** Steven Penny for his generous donations! #286 #251 <a name="thanks"></a>

## Downloads

Stable versions are available via [GitHub releases](https://github.com/ProgerXP/Notepad2e/releases).

Archived non-stable daily builds are available from [this page](http://proger.me/notepad2e/binaries/).

Latest non-stable x86/non-ICU build is permanently available by [this URL](http://proger.me/notepad2e/binaries/LATEST).

## Compilation

1. [Prepare Boost environment](https://github.com/ProgerXP/Notepad2e/blob/master/doc/BoostSetup.md).
    * you will get `libboost_regex-vc141-mt-s-x32-1_68.lib`, `libboost_regex-vc141-mt-s-x64-1_68.lib` in `%BOOST_ROOT%\stage\lib`
2. If you are going to build ICU configurations, [prepare ICU too](https://github.com/ProgerXP/Notepad2e/blob/master/doc/ICUBuild.md). #162
    * you will get the 2 Boost libraries above in `%BOOST_ROOT_ICU%\stage\lib` and also `icuregex64.lib`, `icuregex86.lib`
3. The project comes with a "hacked" Scintilla; if you wish to use the original Scintilla, [read this changelog](https://github.com/ProgerXP/Notepad2e/blob/master/doc/Scintilla_ChangeLog.md).
4. To compile the sources, use Visual Studio 2015 or 2017. #178
5. To run tests (`Notepad2eTests`), point `FileSamplesPath` environment variable to the `...\test\data\Extension` directory. #178

**Note:** x64 configuration is not considered "mainstream" and was poorly tested. #157

## Replacing Windows (XP/7/10) Notepad
One obvious way is to overwrite all `Notepad.exe`s inside Windows directory. However, this irritates SFC and may not persist across OS updates.

A better way is using `Image File Execution Options`, originally explained [here](http://www.flos-freeware.ch/doc/notepad2-Replacement.html). In short: #157

1. Place `Notepad2e.exe` somewhere. `Program Files (x86)\Notepad2e\` directory is a good place.
2. Import this registry key:
```
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\notepad.exe]
"Debugger"="\"C:\\Program Files (x86)\\Notepad2e\\Notepad2e.exe\" /z"
```
  * If the EXE was put elsewhere, edit the part inside the *second* quotes, and don't forget to double all backslashes - example: `...="\"D:\\Foo\\Bar\\MyN2e.exe\" /z"`
  * **Attention:** the EXE should not be named `notepad.exe` (in any char case).

Now whenever Windows needs to launch `Notepad.exe` it will launch the EXE you have specified instead.

To undo, replace last line above with `"Debugger"=-` and import the key.

## INI File Location
*This describes the algorithm used in *Notepad2* and this fork. The process is fully Unicode-safe.*

All settings are stored in a single INI file. If the program cannot find it, then default settings are used and changes to them are lost when the process exits. To make settings persistent, either put [bin\Notepad2e.ini](https://github.com/ProgerXP/Notepad2e/blob/master/bin/Notepad2e.ini) near the EXE or create one from scratch by pressing **F7** (Settings > Save Settings Now).

The following locations are checked for an existing INI file, in order:

1. `/f SOME.INI` command-line switch (relative to program's dir, with possible `%env%` vars). `/f0` forces no INI file even when explicitly asked for one (e.g. by **F7**).
2. `PROGRAM.ini`, where `PROGRAM` is the EXE's name without `.exe` extension, is searched in:
    * Program's directory
    * `%APPDATA%`
    * `%PATH%`
3. Same as above but with `Notepad2.ini` (*Notepad 2e* is a drop-in replacement so its INI file works with *Notepad2* and vice-versa).
4. If an INI was found, it may be further redirected: its `Notepad2.ini` key from `[Notepad2]` section is read and checked:
    * If this key is non-existing or blank, the previously found INI is used
    * Else, if the value is an absolute path (`%env%` vars expanded) to an existing file - it's used as the INI
    * Else, if the path is relative (`%env%` vars expanded) - it's searched in the same folders as `PROGRAM.ini` (above) and used, if found
    * Else, if the key was not blank and no INI was found - the value is used as the (new, non-existing) INI file path (prepended with program's directory if relative)

If the located INI path (`PATH` below) is a directory rather than a file or it ends with `\` then:

1. If `PATH\PROGRAM.ini` exists, it's used as the INI file
2. Else, if `PATH\Notepad2.ini` exists, it's used as the INI file
3. Else, `PATH\PROGRAM.ini` is used anyway

Finally, if the INI's parent directory doesn't exist - it's created.

## Extended Edition Changes
The `[NEW]` mark indicates a new major feature introduced by *Notepad 2e*. Items without this mark are changes (or minor features added) compared to the original *Notepad2*.

Marks of this form: `#123` refer to specific [issues](https://github.com/ProgerXP/Notepad2e/issues) - read them for more details behind a feature.

*Notepad2* has no documentation per se but only [this FAQ](http://www.flos-freeware.ch/development-releases/notepad2-FAQs.html) that explains many interesting features.

### [NEW] Current Word Highlighting
Word under cursor is highlighted in one of 3 modes: #27 #1
1. One occurrence in the document. Indispensable to spot typos.
2. Two or more occurrences but all are visible on the screen.
3. Multiple occurrences with some hidden under the scrollbar.

**Warning:** when word wrapping is enabled, highlighting assumes that entire bottom line is visible even if it has invisible sublines (wrapped continuations).

Related settings:
* `HighlightSelection`
* `MaxSearchDistance`
* More - see **Current Word Highlighting** configuration section.

![Word highlighting types](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/hili-types.gif)

### [NEW] Edit Selection
Allows simultaneous editing of all occurrences of the same string as the selection. Enter the mode with **Ctrl+Tab** (all visible substrings) or **Ctrl+Backtick** (limit substrings to those on the same line). Escape cancels all changes, while Enter or any command that causes cursor to leave the selected block - commits them. #18

**Note:** when selection is empty, this mode affects word near the cursor, and finds other substrings case-insensitively. When selection is non-empty - case-sensitive search is performed, and word boundaries are not checked.

This mode allows easy renaming of variables, typo corrections and so on.

Below, with cursor within `foo` pressing **Ctrl+Tab** will enter this mode and any change you do (such as typing `bar`) will edit all of the three `foo`s at the same time:

```
$foo = "foo";
print($foo);
```

**Warning:** when word wrapping is enabled, highlighting assumes that entire bottom line is visible even if it has invisible sublines (wrapped continuations), thus you might be editing occurrences on those sublines without seeing them.

Related settings:
* See **Current Word Highlighting** section (above).

![Ctrl+Tab - Renaming words on screen](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/ctrl+tab.gif)
![Ctrl+Backtick - Renaming words on line](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/ctrl+backtick.gif)

### [NEW] Math Evaluation
In certain cases (such as in current selection), the file size group in the status bar is replaced with a recognized math expression's result. Left-click on the group copies result to the clipboard (as it appears), right-click toggles the base (bin, oct, dec, hex). #261

* Selection must be shorter than 4096 symbols.
* Evaluation can be restricted to a rectangular selection (Alt+drag). #242
* Expression is locale-insensitive and follows English conventions so that comma is a thousands separator (and ignored) and period is a decimal part separator.
* If expression has commas but no periods and is still valid after replacing commas with periods - then the converted version is evaluated: `12,30 + ,1` = 12.40. This allows more resistance to locale-specific delimiters.
* If result has no fractional part - period and zeros are hidden from the status bar group. Fractional part is discarded if active base is any but decimal.
* The following symbols are ignored: `, $`. Whitespace is ignored unless it separates operands/operators.
* If expression contains `=` then the `=` and everything after it is ignored. Useful for checking calculations: `1+2=4` evaluates to 3.
* Special case: expression with only digits (including hex), radix prefix/suffixes, periods and any ignored symbols (above) is treated as a series of whitespace-separated numbers, which are summed up. For example: `12,3 45.6 $78 10h` = `123+45.6+78+16` = 262.6. #72
  * **Attention:** don't use `e` symbol by itself in this special case (as in scientific notation or as a const), e.g. `1e2` or `1 e 2` - due to implementation nuances, it may not be processed as expect. But it can be part of a hex number: `eh 0xe` = 28. #130

The following expression tokens are recognized:
* operators: `( ) + - * / ^` (caret works as power)
* operators: *div mod shl shr and or xor not*
* hexadecimal base prefix: `0x`
* base suffixes: `b o d h`

Related settings:
* `MathEval`

![Math evaluation](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/math.gif)

### [NEW] Find Next/Previous Word
Vim-like Edit > Find Next/Previous Word (**Ctrl+[Shift]+8**) commands for quick case-insensitive navigation between highlighted words. It's independent of highlight mode settings and of normal Find/Replace and doesn't affect the latter's state. #38

If there's no selection then these search for word at cursor (or for nearby next/previous word). If there's selection then these search for previously used word (not for selection!) starting after last selected symbol.

Related settings:
* `FindWordMatchCase`
* `FindWordWrapAround`

### [NEW] New Line Above/Below
Vim-like Edit > Lines > New Line Above/Below (**Ctrl+Alt+Enter**, **Ctrl+Shift+Enter**) commands that insert a line regardless of which column the cursor is positioned at.

If Auto Indent is enabled and the caret is already at line start/end (whitespace excluded) - indentation of the previous line is used, otherwise - of the current line. #67

### Unindent By Shift+Tab
**Shift+Tab** always unindents selected lines (or current line if empty), even if the caret isn't at line start (exactly as Edit > Block > Unindent). #128

Tab's behaviour is not changed, it still indents to the column. #61

### Find Dialog
* **[NEW]** Find (**Ctrl+F**) now has Grep/Ungrep buttons (can be quickly called with **Alt+G/R**). Find's settings including regexp mode are respected. In case of active selection these commands operate on selected lines only. Big buffers will see a progress bar in the status bar. #46 #29
* **[NEW]** Submitting Find/Replace by **Ctrl+Enter** starts the search from the beginning rather than current position. #273
* Submission buttons are disabled in regexp mode if Search String has errors. #219
* Checkboxes are disabled when certain combinations of flags render other flags invalid (e.g. **regexp** doesn't support **whole word**). #108
* Search and Replace String inputs handle **Ctrl+Backspace** hotkey (delete word before cursor). #121 #50
* **[NEW]** Find and other navigation commands leave certain scroll margin to preserve a customizable amount of lines (such as 33%) above and below the match. Setting: `ScrollYCaretPolicy`. #41 #279
* **[NEW]** The Find icon on the toolbar changes to the Stop icon whenever the search (Find, Replace or Find Word) hits last result in that direction, regardless of the **Wrap around** flag.
* Find respects the **Match case** flag even with Cyrillic characters in the search string. #9
* Fixed *Notepad2* bug: **Shift+F3** called from within Find/Replace must not select text like **Shift+F2**. #275
* Added hints with hotkeys to Find Next, Find Previous and Replace buttons. #274

![Grep](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/grep.gif)
![Ungrep](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/ungrep.gif)

### [NEW] Open Previous
File > Open Previous (**Alt+G**) command lets you toggle between two most recent History files with one keystroke. #8

![Alt+G - Open Previous](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/open-prev.gif)

File > History (**Alt+H**) now pre-selects the target for **Alt+G**: #209
* first item - if current document is not saved ("Untitled")
* second item - in other cases

This allows faster list navigation skipping first and second items (which are directly accessible via **F5** and **Alt+G**).

### [NEW] Open By Prefix
Open Dialog allows opening by prefix - so instead of typing the full file name or selecting a file with your mouse you can only type the name's beginning and hit Enter (or click Open) to open the first matching file. #19

**Note:** prefix cannot match one of reserved file system names such as `NUL` and `CON` (case-insensitive).

Users of Windows 7 and above are advised to enable Explorer autocompletion instead, which offers a similar experience in other system dialogs: #165

```
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\AutoComplete]
"Append Completion"="yes"
```

Related settings:
* `OpenDialogByPrefix`

![Open by prefix](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/open-prefix.gif)

### [NEW] Save On Lose Focus
File > Save On Lose Focus submenu allows automatic saving of the document when program's window loses focus, similarly to Vim's `au FocusLost * :wa`. Saving doesn't occur if any of these is true: #164

* current document is unsaved (*Untitled*), not modified or read-only
* window is not visible (is being closed) or has any dialogs opened (e.g. Tab Settings)
* a child window is opened (e.g. an Open File dialog or Tab Settings)

Related settings:
* `SaveOnLoseFocus`

### [NEW] Open Next/Previous
File > Open Next/Previous commands allow opening files going before/after the currently opened file in its directory #43 based on last used Open/Save File Dialog filter (`Any, *.*` by default, see #258) #277.

These commands have no hotkeys but they can be added as buttons to the toolbar.

**Notes:**

* File position is determined by sorting the file list by name. It doesn't depend on how Explorer or Open Dialog sort files.
* `FindFirstFile()` doesn't support multiple `;`-separated filters like File Dialogs do. If such a filter was used, these buttons will not work.

### Default Save Extension
When saving, new file's extension is determined first by `DefaultExtension` setting (as in *Notepad2*) but if current file was previously opened from or saved to disk then its old extension is used as the default (even if that extension is empty). #17

When saving, if the given new file name ends on period then the file is saved without extension. Example: enter `Makefile.` to get `Makefile` on disk.

### File Dialogs
* **[NEW]** Rename To (**Alt+F6**) command acts as Save As but deletes original file on success. Due to Windows Save File Dialog limitation, it can't be used if new name only differs in character case (an error appears if this is detected). #140
* Open/Save File Dialogs now start with the path of last opened file. #22
* File filter can be changed using [Notepad2's](http://www.flos-freeware.ch/development-releases/notepad2-FAQs.html#ini-file-settings2) `FileDlgFilters` under `[Settings2]`, e.g. `DOC Files (.doc)|.doc|RTF Files (.rtf)|.rtf`. **[NEW]** Last used filter is now saved for Open/Save File Dialogs. #258
* Improved directory locking - in some cases a directory could not be removed even after opening a file in another directory due to Windows' Open File Dialog operation. #100

### Insert Tag (Alt+X)
* Skips leading/trailing whitespace within the selection. This way you don't care about selection precision and can, for example, click on line number to select a full line and wrap it in `<p>` without `</p>` appearing after the line break. #30
* Remembers Opening and Closing tags until the program is closed.
* Supports `{ ( [` in addition to `<` when auto filling Closing tag. Example: `{{#if var}}` produces `{{/if}}`, `[quote]` produces `[/quote]`. #36
* Skips leading non-word symbols when determining tag name: `{{ #if }}` is the same as `{{#if}}` and produces `{{/if}}`, not `{{/}}` as in *Notepad2*.
* Supports HTML5 tags (in addition to XHTML/HTML 4.x tags from *Notepad2*). #120
* Sets **Closing tag** to **Opening tag** if the latter doesn't contain `<` and `>`. Useful when editing certain markup. #24
* In *Notepad2*, Auto Close HTML corrupts non-Latin tags in UTF-8 buffers - this is fixed. #112

![Alt+X - Insert Tag](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/insert-tag.gif)

### Go To (Ctrl+G)
* Extracts first number from Line and Column instead of requiring a strictly numeric value. For example, `abc567.89` navigates to 567. #14
* **[NEW]** Navigation by absolute Offset in the buffer. Respects different character sets to the best effort possible. Value is normalized: #2
  1. Leading symbols that are not `0-9 A-Z a-z` are removed.
  2. If remainder consists of `0-9` then use it as a decimal offset and return.
  3. Remove possible leading `0x` and/or `h`.
  4. If remainder consists of `0-9 A-F a-f` then use it as a hexadecimal offset and return.
  5. In other cases do nothing and don't close the dialog.
* Input priority: use Offset if non-empty, else use Column+Line if Column non-empty, else use only Line.
* Made Go To disabled for an empty buffer. Also made it non-modal and non-combining with Find or Replace. #260
* Added links to/from Find/Replace (preserving Line <=> Search String) and in-dialog hotkeys (**Ctrl+F/H/G**). #259

### File > Launch
* **[NEW]** File > Launch > Shell Menu (**Ctrl+Shift+R**) command invokes Explorer's context menu for currently opened file. Current directory is set to the file's path. Setting: `ShellMenuType`. #12
* **[NEW]** File > Launch > Open Folder (**Ctrl+Alt+L**) command that opens Explorer's window with the current file selected. #136
* File > Launch > Command (**Ctrl+R**) retains the command string until another file is opened and sets current directory to that of the file. #26
* **[NEW]** File > Elevate allows obtaining admin permissions without restarting the program (checked if already elevated, otherwise call it to elevate). #166

![Ctrl+Shift+R - Shell Menu](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/shell-menu.gif)
![Ctrl+Alt+L - Open Folder](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/open-folder.gif)

### Line Selection Hotkeys
* Triple-click and triple-**Ctrl+Space** in Scintilla select entire line. Due to their accidental nature they are disabled in *Notepad 2e*. Full line can still be selected with standard **Ctrl+Shift+Space** hotkey or by clicking on the line number (gutter).
* Single **Ctrl+Space** now selects closest word on the right, ignoring non-word symbols before it. If there is none, entire line sans leading whtiespace is selected. #205

### Line Gutter
* Default gutter style was changed from `size:-2;fore:#ff0000` to `size:-1`.
* Gutter is now automatically resized if it can't fit max line number on the following operations:
  * Reload Settings From Disk
  * Syntax Scheme...
  * 2nd Default Schemes...
  * Customize Schemes...
  * Preview button in Customize Schemes... dialog
  * Default Font...
  * Default lexer setting (**F11**)
  * HTML lexer setting (**Ctrl+F11**)
  * XML lexer setting (**Shift+F11**)
  * Zoom level change (**Ctrl++/-**)
  * File saving
  * Replace/All/In Selection (**Ctrl+H**) #206
  * Drag & drop #222
  * Duplicate Line #237

### PCRE Support
* Replaced incomplete *Notepad2* regexp implementation with a fully-featured Scintilla's Boost Regex - with `(a|b)`, backreferences `\1` (both in Search and Replace Strings) and other features. #90 #114
* One particularly useful feature is ability to change character case with new escape codes: `\l` (one next symbol becomes lower-case), `\L` (all following become lower-case), `\u` and `\U` (similar but for upper-case), `\E` (cancels effect of the preceding `\L` and `\U`). Example: replace from `(.)(.)` to `\l\1\u\2`.
* An ICU version is available, which adds Unicode support to Boost regexps, allowing `\U` and others to work on non-ASCII symbols. #162
    * ICU version's Find/Replace in regexp mode only support Unicode (UTF-8, etc.) buffers. #232 #231
    * Most other national regexp features (`\w`, `\pL`, etc.) work even in non-ICU versions.
* Original Notepad2's regexp didn't support UTF-8 buffers (only ASCII) - Boost's does. #78
* Boost was hacked to allow Replace string to contain NUL (`\0`) - normally it truncates the buffer.
* Another hack was made to allow backward search (Find Previous) in regexp mode. #174
* **Attention:** there are two kinds of backreferences (`\n` and `$n`) and unlike in PHP they are used differently: #145

Backreference | Allowed in Search | Allowed in Replace
--------------|-------------------|-------------------
`\0` | No | No - use `$0` or `$&`
`\n` with n > 0 | Yes | Yes
`$0` | No - `$` = EOL | Yes - alias `$&`
`$n` with n > 0 | No | Yes

Bottomline: use `\n` (n > 0) everywhere except for full-match in Replace - there use `$0`.

### Enclose Selection (Alt+Q)
* Skips leading/trailing whitespace within the selection. For example, enclosing space + `foo` + space produces space + `(foo)` + space instead of `( foo )`.
* **[NEW]** Links to insert Unicode quotes into the focused input. #280
  * US1/2, RU1/2 set quotes according to United States' and Russia's typography standards (see [Wikipedia](https://en.wikipedia.org/wiki/Quotation_mark)).
  * To see Japanese quotes (`「`, etc.) on XP install the Asian support pack (Control Panel > Regional And Language Options > Languages, tick the first checkbox). Later Windows versions come with this preinstalled.
* When "before" string consists of one of these characters: `{ ( [ <` then "after" is set to the same number of `} ) ] >`.
* When "before" consists of one of the characters below then "after" is set to the same string as "before":
```
` ~ ! @ # % ^ * - _ +  = | \ / : ; " ' , . ?
```

These changes make editing Markdown and wiki sources much more pleasant: `[[foo|bar]]`, `**foo bar**`. #37

![Alt+Q - Enclose Selection](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/enclose-sel.gif)

### Wrap/Unwrap Commands
* **[NEW]** Edit > Block > Unwrap Brackes At Cursor (**Ctrl+Shift+3**) command compliments *Notepad2*'s **Ctrl+3-5**. This command removes brackets of type `( { [ <` around the caret (whichever bracket type appears first). Respects nesting. #39
* **[NEW]** Edit > Block > Unwrap Quotes At Cursor (**Ctrl+Shift+4**) command compliments *Notepad2*'s **Ctrl+1-2/6**. This command removes matching `" '` (and backtick) around the caret (text is scanned to the left to determine the quote type). Does not account for nesting or escaping. Multiline.
* Original wrap commands (**Ctrl+1-6**) skip leading/trailing whitespace within the selection.

![Wrap commands](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/wrap.gif)
![Unwrap commands](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/unwrap.gif)

### Special Commands
Split Edit > Special submenu into 2 submenus: Special and Encode. #201

HTML data:
* **[NEW]** Edit > Special > Strip HTML Tags (**Shift+Alt+X**) command removes `<tags>` inside selection, or if there's none - removes first tag before cursor. #40
* **[NEW]** Edit > Special > Escape HTML (**Ctrl+Shift+Alt+X**) command turns `< > &` into `&lt; &gt; &amp;` respectively (inside selection or everywhere in the document if selection is empty). #51 #31

![Shift+Alt+X - String Tags](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/strip-tags.gif)

Binary data:
* **[NEW]** Edit > Encode > String To Hex and Hex To String (**[Ctrl+]Alt+Shift+A**) operate on the document as a bytestream similarly to PHP's `bin2hex()` and `hex2bin()`. Hex To String ignores whitespace (#123). Output: `616263` for `abc`. #87
* **[NEW]** Edit > Encode > QP Encode and QP Decode (**[Ctrl+]Alt+Shift+Q**) operate on the document as a bytestream similarly to PHP's `quoted_printable_encode()` and `quoted_printable_decode()`. Output: `ab=3Dc` for `ab=c`. See [RFC2045, Section 6.7](http://www.faqs.org/rfcs/rfc2045) (this format is typically used in vCards and MIME). #124
* **[NEW]** Edit > Encode > Base64 Encode and Base64 Decode (**[Ctrl+]Alt+Shift+W**) operate on the document as a bytestream similarly to PHP's `base64_encode()` and `base64_decode()`. Base64 Decode ignores whitespace. Output: `YWJj` for `abc`. #122
* Big buffers will see a progress bar in the status bar for the above commands.
* **Warning:** when saving binary data (e.g. a base64-encoded binary file) disable both checkboxes in File > Line Endings > Default, or saved content may be altered. Binary-Safe Save toolbar button does this for you. #170

![Encode/Decode Quoted-Printable/Base64](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/qp-b64.gif)

Other data:
* Edit > Encode > URL Encode and URL Decode (**[Alt+]Ctrl+Shift+E**) use new `UrlEncodeMode` setting for predictable processing according to RFC 3986. #189

### [NEW] Ctrl+Wheel Scroll
Rolling mouse wheel while holding **Ctrl** scrolls the document by entire pages (like **Page Up/Down**) - makes it easier to navigate long scripts. #11 #217

Related settings:
* `WheelScroll`
* `WheelScrollInterval`

### Favorites

* Open Favorites (**Alt+I**) now selects the first item so that **Enter** opens it immediately. #240
* **[NEW]** `/gs start:end` switch sets initial selection by file offsets (see #233). `end` can be `-1` for EOF. #249
  * *Notepad2* has a similar `/g` switch to set initial line by its number.
* **[NEW]** Add To Favorites (**Alt+K**) allows choosing initial line or selection. This affects newly created shortcut (all choices but First Line target *Notepad 2e*'s executable, not the document's file). #249 #282
* **Alt+K**'s initial file name is made unique by appending a counter (like File > Create Desktop Link). #290
* **Alt+K**'s confirmation message is now of type Info on success (not always Warning as in *Notepad2*). #249

### Highlight Line
Ability to keep current line highlighted even if the window is not focused (especially useful when using Windows' X-Mouse feature).

Related settings:
* `HighlightLineIfWindowInactive`

### Drop Text And Files
When dropping an object from another application on an empty line - line break is added automatically. #63 #110

* Useful when creating snippets or lists from other sources - such as from URLs; no more need to manually insert line breaks after each drop.

When dropping a single file, *Notepad2* opens it. **[NEW]** When dropping multiple files, they are concatenated into a fresh (File > New) buffer. #250

* Source files are opened as if using File > Open so unlike with `cat`/`copy` they can use different encoding/line breaks if *Notepad2* can detect them.
* If any file is Unicode then new buffer is made UTF-8 regardless of File > Encoding > Default.

### Copy Add (Ctrl+E)
* Uses single line break instead of double. #28
* When called with empty selection - appends the entire line.
* In *Notepad2*, it didn't work with empty clipboard - now it works.

### File Encoding
* When file is re-coded (File > Encoding menu items) caret position and selection are retained, to the best effort possible. #7
* File > Encoding > Recode (**F8**) renamed to Reload As to better reflect its purpose. #218

### [NEW] Go To Last Change
Go To Last Change (**Ctrl+Shift+Z**) command moves caret to the position of last Undo action. Useful when making a change, scrolling to confirm something and then navigating back to continue. #6

### Join Lines
* Join Lines and Join Paragraphs (**Ctrl+[Shift+]J**) adjust selection's end before the operation so that it doesn't include trailing line break(s). #135
* **[NEW]** On empty selection, Join Lines replaces nearest right-side line break and whitespace around it with a single space and moves the caret after it. This is similar to doing Join Lines + Compress Whitespace on the selection, and to Vim's **J**. #160

### Setting Commands
* **[NEW]** Replace Settings in All Instances command makes all other instances reload the INI from disk. Useful if you have dozens of *Notepad2* windows open and need to change settings in one of them and have them saved (normally when an instance quits it overwrites the INI with its own settings). With this command you can do Save Settings Now, then Replace to ensure your new settings are not overwritten. #5
* **[NEW]** Reload Settings from Disk (**Alt+F7**) command replaces all settings in current window with fresh version read from the INI file. #16
* Save Settings On Exit was extended to allow a third value: Recent Files/Search Strings. This allows you to customize *Notepad 2e* once, Save Settings and then not worry that temporary changes (from a particular instance) will be saved, at the same time allowing the program to save file History and Find/Replace strings. #101
* Save Settings On Exit can be also changed with a toolbar button (toggles between All... and Recent... values). #95

### [NEW] Language Indicator
Window's title reflects current keyboard language, if configured with `TitleLanguage`. For example: `Untitled - Notepad 2e [RU]`. #86

Related settings:
* `TitleLanguage`

![Language indicator](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/lang-title.gif)

### [NEW] Date/Time Indicator

Menu bar can display the clock in the configured format. For example: `07/05/19 - 13:53`. The text is updated every 10 seconds. #210

Related settings:
* `ClockFormat`

![Menu clock](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/menu-clock.png)

### Other Changes
* **[NEW]** File Change Notification is more responsive (suitable for watching log files, like `tail -f`) #129 and reliable (works over network shares; no longer relies on `FindFirstChangeNotification`, only on file time polling) #241.
* Sort Lines (**Alt+O**) and Modify Lines (**Alt+M**) operate on the entire document if selection is empty (*Notepad2* does nothing in this case). #133
* Links of Modify Lines (**Alt+M**) dialog (`$(L)` and others) are inserted into a focused input instead of replacing its value. #119
* All Modify Lines (**Alt+M**) substitutions are replaced, not just first. #271
* Line breaks (`\r` and `\n`) are removed from Search/Replace (of Find/Replace, **Ctrl+F/H**) and Prefix/Append (of Modify Lines, **Alt+M**) inputs (could appear after pasting). #70 #173
* **[NEW]** Now respecting Windows' "Hide pointer while typing" setting in Control Panel > Mouse Properties > Pointer Options. #230
* **[NEW]** Ability to retain caret position and selection on right click. Setting: `MoveCaretOnRightClick`.
* **[NEW]** Displaying selected line count (`L`) in the status bar (in regular and rectangular modes). `B` is for selection length in bytes. #204 #262
* **[NEW]** Displaying current file offset (`Pos`) in the status bar. #233
* **[NEW]** Edit > Insert > Random (**Ctrl+Shift+Alt+R**) inserts a padded number in range 1..99999 (inclusive) using the poor C's `rand()`. #221
* Visual Brace Matching, Find/Select To Matching Brace operate on `< >` too. #283
* "Accelerated" navigation mode for **Ctrl+Arrow** (like in Windows Notepad) that skips punctuation and other characters. Setting: `WordNavigationMode`.
* Empty Window (**Alt+0**) no more triggers save prompt when Save Before Running Tools is enabled. #176
* File > Encoding > UTF-8 has **Shift+F8** hotkey assigned. #21
* File > Line Endings > Unix has **Alt+F8** hotkey assigned. #44
* If large file loading stops due to memory limit, an error message is produced (*Notepad2* silently stops loading it). #126
* **[NEW]** DPI awareness - proper font scaling with crisp texts without Windows fallback mechanism. #154
* Fixed bug of *Notepad2* in processing `[Toolbar Labels]` INI section. #150
* Upgraded Scintilla library to a more recent version (~~3.6.6~~ 3.11.2).
* Added `<supportedOS>` manifest entries for Windows 10/8.1/8 (Server 2016/2012/R2), in addition to Windows 7/Vista (Server 2008/R2). #159
* Reduced default *Notepad2* timeout from 1000 ms to 250 ms which sometimes allowed duplicate windows even if Single File Instance/Reuse Window were enabled. #177
* Changed hardcoded printed page's header/footer font size from 8 pt/7 pt to 10 pt. #199
* Grouped commands into submenus in Edit and Settings. #265
* Reorganized `&` accelerators in File, Edit and Settings. #197 #276
* Disabled commands when no file is opened: File > Open Next/Previous and Launch > Shell Menu #229; Launch > Open With #238; Edit > Strip/Escape HTML Tags, Find Next/Previous Word, Edit Selection/On Line #268.
* **[NEW]** **?** > 3rd-Party Code attribution dialog. #181
* Changed *Notepad2* defaults: #167

Setting | Old Value | New Value
--------|-----------|----------
Long Line | 72 | 80

### Undocumented Notepad2 Features
* Rectangular selection mode is actually supported - hold **Alt** while dragging your mouse to make a selection. This is particularly useful for **Column sort** in Sort Lines (**Alt+O**).
* Web search: if you set `WebTemplate1` (or `WebTemplate2`) setting in the INI to `https://google.com/search?q=%s` and then press **Ctrl+Shift+1-2** with non-empty selection - you will be navigated to that URL (`%s` replaced with a selection string, not URL-encoded).
* If Replace's dialog Replace With is `^c` - clipboard contents is used instead of this string.
* If the program is started with `/B` flag, it enters "Pasteboard mode" where new content on the clipboard is automatically added to the buffer.
* `[Custom Colors]` INI section is used to fill the "Custom colors" control of Fore/Back color pickers in Customize Schemes. #149

![Rectangular selection and column sorting](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/sort-column.gif)
![Pasteboard mode](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/pasteboard.gif)

### Syntax Schemes

**[NEW]** These syntax schemes were added:
* AutoHotKey (AHK) #214
* `awk` #216
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
* Related setting: `CSSSettings`.

HTML syntax scheme was improved:
* Added HTML5 tags. #236

Lua syntax scheme was improved:
* Added single-line: `--...` and multi-line comments: `--[[...]]`. #111


## [NEW] Lua Lexers #251
<a name="lpeg"></a>

Scintilla's [LPegLexer](https://scintilla.sourceforge.io/LPegLexer.html) was enabled to allow defining new syntax schemes in Lua.

**Warning:** Lua is a fully functional language so allowing untrusted files inside `LPegPath` **IS A VERY BAD IDEA** as it allows execution of arbitrary code via Notepad 2e.

* LPeg is not available in default Notepad 2e versions - download the one with LPeg support (or with LPeg + ICU if you want both although ICU doesn't affect LPeg).
* LPeg support adds ~400 KiB to the EXE file size.
* With LPeg, Notepad 2e is no longer single-file distribution and requires certain Lua files to exist in the location configured by the `LPegPath` INI setting (that is empty by default, disabling support for LPeg).

Configuration guide:

* Define `LPegPath` under `[Notepad2e]` in your `Notepad2e.ini` file. Its value is relative to the EXE location and can contain `%variables%`.
```
[Notepad2e]
LPegPath=lexlua
```
* Once done, open a new Notepad 2e window. It should have created files and folders under `LPegPath` (so it needs write access unless they already exist).
  * If you see an error message about disabled LPeg support on start-up then you failed to configure `LPegPath`.
* For every new syntax scheme you want to support, copy the lexer file to `LPegPath` under the name *file_extension + `.lua`*. For example: `pas.lua` for `*.pas` files. Then, open View > Customize Schemes (Ctrl+F12) and select `LPeg Lexer` in the end of the list and enter all extensions you want to be handled by LPeg (e.g. `pas;dpr`).
  * If you don't see `LPeg Lexer` in the list then your Notepad 2e version wasn't built with LPeg support.
* If you want LPeg to be picked automatically when you open a file then check *Auto-select* for `LPeg Lexer` under View > Syntax Scheme (F12; it's checked by default) and make sure other schemes don't have extensions listed for `LPeg Lexer`. Else you can uncheck it and be picking LPeg manually (but you still need extensions configured - LPeg will render others as Default Text).
* If Notepad 2e tries to highlight a file and there's no lexer then you'll see an error saying which file you need to supply.

In the end, your `LPegPath` should contain these files:

```
themes\default.lua    - required, created by Notepad 2e if missing
lexer.lua             - required, created by Notepad 2e if missing
pas.lua               - lexer for *.pas files, created by you
...                   - other lexers
```

* `themes\` defines display colors. Notepad 2e doesn't support switching themes so it only uses `default.lua`.
* We don't provide support for LPeg itself so to see how these lexers are written go to the [project's homepage](https://scintilla.sourceforge.io/LPegLexer.html).
* Pre-made lexers are available from [Scintilla 3's source code archive](https://scintilla.sourceforge.io/LongTermDownload.html) (the `lexlua` folder, but you need to rename files in accordance to file extensions they highlight).
* Also see the original [project's page](https://foicica.com/wiki/scintillua).


## Extended Edition INI Changes
Settings in this section extend *Notepad2*'s values and thus appear under its own INI groups, normally under `[Settings]`.

### SaveSettings

Type | Default | Set By UI
-----|---------|----------
int | 1 (if INI exists) | Settings > Save Settings On Exit

Causes the program to save its settings on exit. Any changes to the INI file from the time this process was started are overwritten.

Saving is only possible when an INI file exists. The easiest way to create it is using Save Settings Now (**F7**) command.

Value | Meaning
----|-----
0 | Don't save settings
1 | Save all settings
2 | **[NEW]** Save only Recent Files and Search Strings but no settings


## [NEW] Extended Edition INI Configuration
*Notepad2* stores all of its settings in `Notepad2.ini` (or rather in `exe_base_name.ini`).
*Notepad 2e* uses the same file but puts its settings under the `[Notepad2e]` group.

### DebugLog

Type | Default | Set By UI
-----|---------|----------
int, bool | 0 |

Enables creation of debug log file `n2e_log.log` in the program's folder.

### CSSSettings

Type | Default | Set By UI
-----|---------|----------
int, bitfield | 2 |

Extend standard CSS highlighting to support:

Bit | Type
----|-----
1 | Sassy
2 | LESS
4 | HSS

It's a bitfield so bits can be combined: **3** = Sassy + LESS.

### FindWordMatchCase & FindWordWrapAround

Type | Default | Set By UI
-----|---------|----------
int, bool | 0 |

These control **Ctrl[+Shift]+8** search like normal Find dialog flags.

The **Match whole word only** flag is always enabled for those commands so it can't be customized.

### HighlightLineIfWindowInactive

Type | Default | Set By UI
-----|---------|----------
int, bool | 0 |

If **0**, current line is stops being highlighted when window is inactive (default *Notepad2* behaviour).

If **1**, highlighting is independent of window focus (always visible if enabled).

### OpenDialogByPrefix

Type | Default | Set By UI
-----|---------|----------
int | 1 |

Indicates if Open File dialog can be submitted even if just a prefix of an existing file's name was entered.

Windows 7+ have a registry preference that enables autocompletion in various places (Win+R, Open/Save dialogs, Explorer windows, etc.). Its effect on Open File dialogs is similar to this *Notepad 2e* feature. Enabling both doesn't cause any trouble but it makes more sense to have only one of them active. #165

Value | Meaning
------|--------
0 | Don't enable, use native Windows behaviour
1 | Enable unless Explorer autocompletion is enabled
2 | Always enable

### ScrollYCaretPolicy

Type | Default | Set By UI
-----|---------|----------
int | 0 |

Sets vertical margin for commands that can scroll the buffer, including:
* **F3, F2, Ctrl+8** and their **Shift** versions
* **Ctrl+], Ctrl+[** and their **Shift** versions
* **Page Up, Page Down** and their **Shift**, **Alt+Shift** versions
- **Ctrl+Alt+Shift+Z** #279

Value | Meaning
------|--------
0 | Margin of 4 extra lines (as in *Notepad2*)
1 | 33% margin
2 | 50% margin (central line)

### MathEval

Type | Default | Set By UI
-----|---------|----------
int | 1 | Settings > Evaluate Math Expressions

Controls math expression evaluation. #88

Value | Meaning
------|--------
0 | No evaluation (as in *Notepad2*)
1 | Evaluate selection or, if empty - entire current line (if it's a valid expression)
2 | Evaluate selection only (if it's a valid expression)

### TitleLanguage

Type | Default | Set By UI
-----|---------|----------
int | 2 | Settings > Window Title Display

Controls keyboard language display in window's title.

Value | Meaning
------|--------
0 | No indication (as in *Notepad2*)
1 | Always add language name as in `... [RU]`
2 | As **1** but don't add if the language is English (`EN`)

### ClockFormat

Type | Default | Set By UI
-----|---------|----------
str, `strftime()` | |

Controls format of the date/time indicator in the menu bar. If empty, it's hidden.

Value is a standard `strftime()` format string composed of `%smth` sequences to be replaced by the corresponding values. Useful variables:

Variable | Meaning | Example
---------|---------|--------
%% | The percent symbol itself | %
%Y | Year | 2019
%y | Short year | 19
%B | Month name | July
%b | Short month name | Jul
%m | Month | 07
%V | Week of the year | 27
%j | Day of the year | 186
%e | Day | 5
%A | Weekday name | Friday
%a | Short weekday name | Fri
%H | Hour in 24-hour format | 14
%I | Hour in 12-hour format | 02
%p | Locale-dependent A.M. or P.M. | P.M.
%M | Minute | 03
%S | Second | 25
%c | Locale-dependent standard date/time | Fri Jul 5 14:03:25 2019
%x | Locale-dependent standard date | 07/05/19
%X | Locale-dependent standard time | 14:03:25
%Z | Locale-dependent time zone | Russia TZ 2 Standard Time

Full reference: https://en.cppreference.com/w/cpp/chrono/c/strftime

### ShellMenuType

Type | Default | Set By UI
-----|---------|----------
int, bitfield | 0 |

Controls behaviour of Shell Menu (**Ctrl+Shift+R**). For values see `uFlags` at this page:
http://msdn.microsoft.com/en-us/library/windows/desktop/bb776097(v=vs.85).aspx

These flags affect how the menu looks like and which commands are available; it's highly dependent on the OS and particular environment. Useful values you may try in case of problems (mainly on Windows XP):
* `ShellMenuType=16`
* `ShellMenuType=4`

Bit | Type
----|-----
0x00000000 | NORMAL
0x00000001 | DEFAULTONLY
0x00000002 | VERBSONLY
0x00000004 | EXPLORE
0x00000008 | NOVERBS
0x00000010 | CANRENAME
0x00000020 | NODEFAULT
0x00000040 | INCLUDESTATIC
0x00000080 | ITEMMENU
0x00000100 | EXTENDEDVERBS
0x00000200 | DISABLEDVERBS
0x00000400 | ASYNCVERBSTATE
0x00000800 | OPTIMIZEFORINVOKE
0x00001000 | SYNCCASCADEMENU
0x00002000 | DONOTPICKDEFAULT
0xFFFF0000 | RESERVED

### WheelScroll

Type | Default | Set By UI
-----|---------|----------
int, bool | 1 | Settings > Ctrl+Wheel Scroll

If **0** then **Ctrl+Wheel** changes zoom level (*Notepad2* behaviour).

If **1** then **Ctrl+Wheel** scrolls the document by entire pages.

### WheelScrollInterval

Type | Default | Set By UI
-----|---------|----------
int, ms | 50 ms |

When using **Ctrl+Wheel**, buffer will be scrolled at most once per this interval. Edit it if the program skips wheel rotations or is too sensitive.

Necessary because Windows fires a handful of wheel scroll events per one real scroll.

### MoveCaretOnRightClick

Type | Default | Set By UI
-----|---------|----------
int, bool | 1 | Settings > Move Caret On Right Click

If **0** - caret is not moved and selection is not changed on right mouse button click. #54

![Not moving caret on right-click](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/no-move-context.gif)

### WordNavigationMode

Type | Default | Set By UI
-----|---------|----------
int, bool | 0 | Settings > Ctrl+Arrow Navigation

Controls **Ctrl+Arrow** navigation. If **1**, enables "accelerated" mode where only whitespace is considered a word boundary, not punctuation, brackets, etc. (useful when working with natural language texts, not program code). #89 #156

![Accelerated navigation](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/nav-accel.gif)
![Standard navigation](https://github.com/ProgerXP/Notepad2e/raw/master/doc/gif/nav-std.gif)

### UrlEncodeMode

Type | Default | Set By UI
-----|---------|----------
int, bool | 1 |

If **0**, use *Notepad2*'s behaviour calling [UrlEscape](https://docs.microsoft.com/en-us/windows/desktop/api/shlwapi/nf-shlwapi-urlescapea) with `URL_ESCAPE_SEGMENT_ONLY`.

If **1**, perform full URL encoding/decoding per RFC 3986.

### SaveOnLoseFocus

Type | Default | Set By UI
-----|---------|----------
int | 0 | File > Save On Lose Focus

Controls if current document is saved when window loses focus, except under certain conditions.

Value | Meaning
------|--------
0 | Don't save
1 | Save
2 | Save, and when a new file is opened or created, reset this setting to **0**

### LPegPath

Type | Default | Set By UI
-----|---------|----------
str, path | *(empty)* |

Enables [LPeg Lexer support](#lpeg). Ignored in non-LPeg versions. If empty, Lua is disabled.

### Current Word Highlighting

Settings in this section that begin with `_` have variations depending on highlighting conditions; all variations have the same format and meaning but may have different default values and apply in different situations.

Prefix   | Conditions
---------|----------
*(none)* | Multiple occurrences, some are not visible on screen
Page     | Multiple occurrences, all are visible on screen
Single   | Single occurrence
Edit     | Edit Mode active

For example, `_SelectionType` expands to the following settings:
```
SelectionType
PageSelectionType
SingleSelectionType
EditSelectionType
```

#### HighlightSelection

Type | Default | Set By UI
-----|---------|----------
int, bool | 1 | View > Highlight Current Word

If **1**, enables both current word highlighting and Edit Mode. Edit Mode doesn't work if highlighting is disabled.

#### MaxSearchDistance

Type | Default | Set By UI
-----|---------|----------
int, KiB | 96 KiB |

Maximum lookahead/behind distance for word highlighting. If too large, navigation in big files will lag since it will search the buffer for twice this length (back & forward) on every position change. #53 #42

#### _SelectionType

Type | Default | Set By UI
-----|---------|----------
int | 7 (`INDIC_ROUNDBOX` - `Page`, `Edit`), 6 (`INDIC_BOX` - others) |

Decoration type. Value **5** disables this condition indication (other settings are ignored) - can be used, for example, to disable special rendering of `Single` occurrence.

* For `Edit` (and other modes), **5** disables indication only - Edit Mode still continues to work. Note that value **0** doesn't disable indication as one might think (see the table below and #146 for the why).
* If you want to disable highlighting and Edit Mode then use `HighlightSelection` setting as it removes all highlighting overhead.
* Adjust search distance (performance on large buffers) with `MaxSearchDistance`.

For details and examples see:
http://www.scintilla.org/ScintillaDoc.html#Indicators

Symbol | Value
-------|------
INDIC_PLAIN | 0
INDIC_SQUIGGLE | 1
INDIC_TT | 2
INDIC_DIAGONAL | 3
INDIC_STRIKE | 4
INDIC_HIDDEN | 5
INDIC_BOX | 6
INDIC_ROUNDBOX | 7
INDIC_STRAIGHTBOX | 8
INDIC_FULLBOX | 16
INDIC_DASH | 9
INDIC_DOTS | 10
INDIC_SQUIGGLELOW | 11
INDIC_DOTBOX | 12
INDIC_SQUIGGLEPIXMAP | 13
INDIC_COMPOSITIONTHICK | 14
INDIC_COMPOSITIONTHIN | 15
INDIC_TEXTFORE | 17
INDIC_POINT | 18
INDIC_POINTCHARACTER | 19

#### _SelectionAlpha

Type | Default | Set By UI
-----|---------|----------
int | 50 (`Page`, `Edit`), 0 (others) |

Opacity value (0-255) for foreground highlight color.

#### _SelectionLineAlpha

Type | Default | Set By UI
-----|---------|----------
int | 255 (`Page`, `Edit`), 0 (others) |

Opacity value (0-255) for highlight outline color.

#### _SelectionColor

Type | Default | Set By UI
-----|---------|----------
str, BGR | Varies (below) |

Condition | Default (RGB)
----------|--------
*Multiple invisible* | `#00AA00`
Page      | `#999900`
Single    | `#900000`
Edit      | `#0000FF`

Foreground highlight color like `0xFF0000` (blue - not RGB!).

#### _SelectionUnder

Type | Default | Set By UI
-----|---------|----------
int, bool | 1 (under - `Page`, `Edit`), 0 (over - others) |

Corresponds to Scintilla's [SCI_INDICSETUNDER](http://www.scintilla.org/ScintillaDoc.html#SCI_INDICSETUNDER):

> [...] whether an indicator is drawn under text or over (default).
