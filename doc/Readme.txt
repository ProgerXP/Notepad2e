=======================================================================
=                                                                     =
=                                                                     =
=  Notepad 2e - light-weight Scintilla-based text editor for Windows  =
=                                                                     =
=                                                                     =
=                                                    Notepad2 4.2.25  =
=                                       (c) Florian Balmer 2004-2011  =
=                                        http://www.flos-freeware.ch  =
=                                                                     =
=  Extended edition (c) 2013-2019                                     =
=                                                                     =
=                                      by Proger_XP and contributors  =
=                              https://github.com/ProgerXP/Notepad2e  =
=                                                                     =
=======================================================================


The Notepad2e Source Code

  This package contains the full source code of Notepad2e for
  Windows. Project files for Visual Studio 2017 are included.

  For the original readme.txt of Notepad2, see Notepad2.txt.


Rebuilding from the Source Code

  Notepad2e is based on Scintilla 2.24 [1] which is already included in
  the repository..

  [1] http://www.scintilla.org

  Additionally, Notepad2e requires boost to be set up, see BoostSetup.md.

  For building Notepad2e, just open Notepad2e.sln and build it.

  For ICU support see ICUBuild.md.


How to add or modify Syntax Schemes

  The Scintilla documentation has an overview of syntax highlighting,
  and how to write your own lexing module, in case the language you
  would like to add is not currently supported by Scintilla.

  Add your own lexer data structs to the global pLexArray (Styles.c),
  then adjust NUMLEXERS (Styles.h) to the new total number of syntax
  schemes. Include the "scintilla/lexers/Lex*.cxx" file required for
  your language into your project. Ensure the new module is initialized
  (in "scintilla/src/Catalogue.cxx"), either by manually uncommenting
  the corresponding LINK_LEXER() macro call, or by updating and
  re-running LinkLex.js.

  Many of the Scintilla lexing modules are not used by Notepad2e. Run
  LinkLex.js to adapt the list (in "scintilla/src/Catalogue.cxx") and
  make linking work properly.


Copyright

  See License.txt for details about distribution and modification.
  
  If you have any comments or questions, please drop me a note:
  florian.balmer@gmail.com

  (c) Florian Balmer 2004-2011
  http://www.flos-freeware.ch

  (c) Proger_XP and co. 2013-2019
  http://proger.me

###
