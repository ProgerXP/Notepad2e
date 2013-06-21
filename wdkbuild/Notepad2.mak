.SILENT :

!IF "$(CFG)" != "i386" && "$(CFG)" != "amd64"
CFG=i386
!ENDIF

!IF "$(CFG)" == "i386"
OUTPUT=..\Release\i386
CFLAGS=/c /EHsc /GF /GS- /MD /O1 /W3
RCCFG=/d "_M_IX86"
LDCFG=/VERSION:5.0 /SUBSYSTEM:WINDOWS,5.0 /OSVERSION:5.0 /MACHINE:IX86 /NXCOMPAT /DYNAMICBASE
LIBS2=msvcrt_win2000.obj
!ELSE
OUTPUT=..\Release\amd64
CFLAGS=/c /EHsc /GF /GS- /MD /O1 /W3
RCCFG=/d "_M_AMD64"
LDCFG=/VERSION:5.2 /SUBSYSTEM:WINDOWS,5.2 /OSVERSION:5.2 /MACHINE:AMD64 /NXCOMPAT /DYNAMICBASE
LIBS2=msvcrt_win2003.obj
!ENDIF

CC=cl /nologo
RC=rc
LD=link /nologo

SRC=..\src
RES=..\res
SCI=..\scintilla
EXE=$(OUTPUT)\Notepad2.exe

CDEF=/D "STATIC_BUILD" /D "SCI_LEXER" /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_UNICODE" /D "UNICODE"
CINC=/I "$(SCI)\include" /I "$(SCI)\lexlib" /I "$(SCI)\src" /I "$(SCI)\win32"
RCFLAGS=/l 0x0409 /d "NDEBUG"
LDFLAGS=/INCREMENTAL:NO /RELEASE /OPT:REF /OPT:ICF /MERGE:.rdata=.text
LIBS= \
  kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib shlwapi.lib comdlg32.lib \
  comctl32.lib winspool.lib imm32.lib ole32.lib oleaut32.lib psapi.lib

BUILD : PREBUILD $(EXE) POSTBUILD

PREBUILD :
  echo Building...
  -md "$(OUTPUT)" >nul 2>&1

POSTBUILD :
  echo Success.

CLEAN :
  echo Cleaning...
  -del /s "$(OUTPUT)\*.exe" "$(OUTPUT)\*.obj" "$(OUTPUT)\*.res" "$(OUTPUT)\*.pdb" "$(OUTPUT)\*.idb" >nul 2>&1

OBJ= \
  $(OUTPUT)\LexAsm.obj \
  $(OUTPUT)\LexConf.obj \
  $(OUTPUT)\LexCPP.obj \
  $(OUTPUT)\LexCSS.obj \
  $(OUTPUT)\LexHTML.obj \
  $(OUTPUT)\LexOthers.obj \
  $(OUTPUT)\LexPascal.obj \
  $(OUTPUT)\LexPerl.obj \
  $(OUTPUT)\LexPowerShell.obj \
  $(OUTPUT)\LexPython.obj \
  $(OUTPUT)\LexSQL.obj \
  $(OUTPUT)\LexVB.obj \
  $(OUTPUT)\Accessor.obj \
  $(OUTPUT)\CharacterSet.obj \
  $(OUTPUT)\LexerBase.obj \
  $(OUTPUT)\LexerModule.obj \
  $(OUTPUT)\LexerSimple.obj \
  $(OUTPUT)\PropSetSimple.obj \
  $(OUTPUT)\StyleContext.obj \
  $(OUTPUT)\WordList.obj \
  $(OUTPUT)\AutoComplete.obj \
  $(OUTPUT)\CallTip.obj \
  $(OUTPUT)\Catalogue.obj \
  $(OUTPUT)\CellBuffer.obj \
  $(OUTPUT)\CharClassify.obj \
  $(OUTPUT)\ContractionState.obj \
  $(OUTPUT)\Decoration.obj \
  $(OUTPUT)\Document.obj \
  $(OUTPUT)\Editor.obj \
  $(OUTPUT)\ExternalLexer.obj \
  $(OUTPUT)\Indicator.obj \
  $(OUTPUT)\KeyMap.obj \
  $(OUTPUT)\LineMarker.obj \
  $(OUTPUT)\PerLine.obj \
  $(OUTPUT)\PositionCache.obj \
  $(OUTPUT)\RESearch.obj \
  $(OUTPUT)\RunStyles.obj \
  $(OUTPUT)\ScintillaBase.obj \
  $(OUTPUT)\Selection.obj \
  $(OUTPUT)\Style.obj \
  $(OUTPUT)\UniConversion.obj \
  $(OUTPUT)\ViewStyle.obj \
  $(OUTPUT)\XPM.obj \
  $(OUTPUT)\PlatWin.obj \
  $(OUTPUT)\ScintillaWin.obj \
  $(OUTPUT)\Dialogs.obj \
  $(OUTPUT)\Dlapi.obj \
  $(OUTPUT)\Edit.obj \
  $(OUTPUT)\Helpers.obj \
  $(OUTPUT)\Notepad2.obj \
  $(OUTPUT)\Styles.obj \
  $(OUTPUT)\Print.obj \
  $(OUTPUT)\Notepad2.res

{$(SCI)\lexers\}.cxx{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

{$(SCI)\lexlib}.cxx{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

{$(SCI)\src}.cxx{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

{$(SCI)\win32}.cxx{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

{$(SRC)\}.c{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

{$(SRC)\}.cpp{$(OUTPUT)}.obj ::
  $(CC) /Fo$(OUTPUT)\ $(CINC) $(CDEF) $(CFLAGS) $<

$(OUTPUT)\Notepad2.res : $(SRC)\Notepad2.rc
  echo $(**F)
  $(RC) $(RCFLAGS) $(RCCFG) /fo$@ $** >nul

$(EXE) : $(OBJ)
  $(LD) /OUT:"$(EXE)" $(LDFLAGS) $(LDCFG) $(LIBS) $** $(LIBS2)

$(OUTPUT)\LexAsm.obj : \
  $(SCI)\lexers\LexAsm.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexConf.obj : \
  $(SCI)\lexers\LexConf.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexCPP.obj : \
  $(SCI)\lexers\LexCPP.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\lexlib\OptionSet.h

$(OUTPUT)\LexCSS.obj : \
  $(SCI)\lexers\LexCSS.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexHTML.obj : \
  $(SCI)\lexers\LexHTML.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexOthers.obj : \
  $(SCI)\lexers\LexOthers.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexPascal.obj : \
  $(SCI)\lexers\LexPascal.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexPerl.obj : \
  $(SCI)\lexers\LexPerl.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexPowerShell.obj : \
  $(SCI)\lexers\LexPowerShell.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexPython.obj : \
  $(SCI)\lexers\LexPython.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexSQL.obj : \
  $(SCI)\lexers\LexSQL.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\LexVB.obj : \
  $(SCI)\lexers\LexVB.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\lexlib\LexerModule.h

$(OUTPUT)\Accessor.obj : \
  $(SCI)\lexlib\Accessor.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\PropSetSimple.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h

$(OUTPUT)\CharacterSet.obj : \
  $(SCI)\lexlib\CharacterSet.cxx \
  $(SCI)\lexlib\CharacterSet.h

$(OUTPUT)\LexerBase.obj : \
  $(SCI)\lexlib\LexerBase.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\PropSetSimple.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\lexlib\LexerBase.h

$(OUTPUT)\LexerModule.obj : \
  $(SCI)\lexlib\LexerModule.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\PropSetSimple.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\lexlib\LexerBase.h \
  $(SCI)\lexlib\LexerSimple.h

$(OUTPUT)\LexerSimple.obj : \
  $(SCI)\lexlib\LexerSimple.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\PropSetSimple.h \
  $(SCI)\lexlib\WordList.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\lexlib\LexerBase.h \
  $(SCI)\lexlib\LexerSimple.h

$(OUTPUT)\PropSetSimple.obj : \
  $(SCI)\lexlib\PropSetSimple.cxx \
  $(SCI)\lexlib\PropSetSimple.h

$(OUTPUT)\StyleContext.obj : \
  $(SCI)\lexlib\StyleContext.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\lexlib\LexAccessor.h \
  $(SCI)\lexlib\Accessor.h \
  $(SCI)\lexlib\StyleContext.h

$(OUTPUT)\WordList.obj : \
  $(SCI)\lexlib\WordList.cxx \
  $(SCI)\lexlib\WordList.h

$(OUTPUT)\AutoComplete.obj : \
  $(SCI)\src\AutoComplete.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\src\AutoComplete.h

$(OUTPUT)\CallTip.obj : \
  $(SCI)\src\CallTip.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\CallTip.h

$(OUTPUT)\Catalogue.obj : \
  $(SCI)\src\Catalogue.cxx \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\src\Catalogue.h

$(OUTPUT)\CellBuffer.obj : \
  $(SCI)\src\CellBuffer.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\CellBuffer.h

$(OUTPUT)\CharClassify.obj : \
  $(SCI)\src\CharClassify.cxx \
  $(SCI)\src\CharClassify.h

$(OUTPUT)\ContractionState.obj : \
  $(SCI)\src\ContractionState.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\ContractionState.h

$(OUTPUT)\Decoration.obj : \
  $(SCI)\src\Decoration.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\Decoration.h

$(OUTPUT)\Document.obj : \
  $(SCI)\src\Document.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\PerLine.h \
  $(SCI)\src\CharClassify.h \
  $(SCI)\lexlib\CharacterSet.h \
  $(SCI)\src\Decoration.h \
  $(SCI)\src\Document.h \
  $(SCI)\src\RESearch.h \
  $(SCI)\src\UniConversion.h

$(OUTPUT)\Editor.obj : \
  $(SCI)\src\Editor.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\ContractionState.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\KeyMap.h \
  $(SCI)\src\Indicator.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h \
  $(SCI)\src\Style.h \
  $(SCI)\src\ViewStyle.h \
  $(SCI)\src\CharClassify.h \
  $(SCI)\src\Decoration.h \
  $(SCI)\src\Document.h \
  $(SCI)\src\Selection.h \
  $(SCI)\src\PositionCache.h \
  $(SCI)\src\Editor.h

$(OUTPUT)\ExternalLexer.obj : \
  $(SCI)\src\ExternalLexer.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\src\Catalogue.h \
  $(SCI)\src\ExternalLexer.h

$(OUTPUT)\Indicator.obj : \
  $(SCI)\src\Indicator.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\Indicator.h

$(OUTPUT)\KeyMap.obj : \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\KeyMap.h

$(OUTPUT)\LineMarker.obj : \
  $(SCI)\src\LineMarker.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h

$(OUTPUT)\PerLine.obj : \
  $(SCI)\src\PerLine.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\PerLine.h

$(OUTPUT)\PositionCache.obj : \
  $(SCI)\src\PositionCache.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\ContractionState.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\KeyMap.h \
  $(SCI)\src\Indicator.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h \
  $(SCI)\src\Style.h \
  $(SCI)\src\ViewStyle.h \
  $(SCI)\src\CharClassify.h \
  $(SCI)\src\Decoration.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\src\Document.h \
  $(SCI)\src\Selection.h \
  $(SCI)\src\PositionCache.h

$(OUTPUT)\RESearch.obj : \
  $(SCI)\src\RESearch.cxx \
  $(SCI)\src\CharClassify.h \
  $(SCI)\src\RESearch.h

$(OUTPUT)\RunStyles.obj : \
  $(SCI)\src\RunStyles.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h

$(OUTPUT)\ScintillaBase.obj : \
  $(SCI)\src\ScintillaBase.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\lexlib\PropSetSimple.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\src\Catalogue.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\ContractionState.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\CallTip.h \
  $(SCI)\src\KeyMap.h \
  $(SCI)\src\Indicator.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h \
  $(SCI)\src\Style.h \
  $(SCI)\src\ViewStyle.h \
  $(SCI)\src\AutoComplete.h \
  $(SCI)\src\CharClassify.h \
  $(SCI)\src\Decoration.h \
  $(SCI)\src\Document.h \
  $(SCI)\src\Selection.h \
  $(SCI)\src\PositionCache.h \
  $(SCI)\src\Editor.h \
  $(SCI)\src\ScintillaBase.h

$(OUTPUT)\Selection.obj : \
  $(SCI)\src\Selection.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\Selection.h

$(OUTPUT)\Style.obj : \
  $(SCI)\src\Style.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\Style.h

$(OUTPUT)\UniConversion.obj : \
  $(SCI)\src\UniConversion.cxx \
  $(SCI)\src\UniConversion.h

$(OUTPUT)\ViewStyle.obj : \
  $(SCI)\src\ViewStyle.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\Indicator.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h \
  $(SCI)\src\Style.h \
  $(SCI)\src\ViewStyle.h

$(OUTPUT)\XPM.obj : \
  $(SCI)\src\XPM.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\src\XPM.h

$(OUTPUT)\PlatWin.obj : \
  $(SCI)\win32\PlatWin.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\src\UniConversion.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\FontQuality.h

$(OUTPUT)\ScintillaWin.obj : \
  $(SCI)\win32\ScintillaWin.cxx \
  $(SCI)\include\Platform.h \
  $(SCI)\include\ILexer.h \
  $(SCI)\include\Scintilla.h \
  $(SCI)\include\SciLexer.h \
  $(SCI)\lexlib\LexerModule.h \
  $(SCI)\src\SplitVector.h \
  $(SCI)\src\Partitioning.h \
  $(SCI)\src\RunStyles.h \
  $(SCI)\src\ContractionState.h \
  $(SCI)\src\CellBuffer.h \
  $(SCI)\src\CallTip.h \
  $(SCI)\src\KeyMap.h \
  $(SCI)\src\Indicator.h \
  $(SCI)\src\XPM.h \
  $(SCI)\src\LineMarker.h \
  $(SCI)\src\Style.h \
  $(SCI)\src\AutoComplete.h \
  $(SCI)\src\ViewStyle.h \
  $(SCI)\src\CharClassify.h \
  $(SCI)\src\Decoration.h \
  $(SCI)\src\Document.h \
  $(SCI)\src\Selection.h \
  $(SCI)\src\PositionCache.h \
  $(SCI)\src\Editor.h \
  $(SCI)\src\ScintillaBase.h \
  $(SCI)\src\UniConversion.h \
  $(SCI)\src\ExternalLexer.h

$(OUTPUT)\Dialogs.obj : \
  $(SRC)\Dialogs.c \
  $(SRC)\Notepad2.h \
  $(SCI)\include\scintilla.h \
  $(SRC)\Edit.h \
  $(SRC)\Helpers.h \
  $(SRC)\Dlapi.h \
  $(SRC)\Dialogs.h \
  $(SRC)\resource.h

$(OUTPUT)\Dlapi.obj : \
  $(SRC)\Dlapi.c \
  $(SRC)\Dlapi.h

$(OUTPUT)\Edit.obj : \
  $(SRC)\Edit.c \
  $(SRC)\Notepad2.h \
  $(SRC)\Helpers.h \
  $(SRC)\Dialogs.h \
  $(SCI)\include\scintilla.h \
  $(SCI)\include\scilexer.h \
  $(SRC)\Styles.h \
  $(SRC)\Edit.h \
  $(SRC)\resource.h

$(OUTPUT)\Helpers.obj : \
  $(SRC)\Helpers.c \
  $(SRC)\Helpers.h \
  $(SRC)\resource.h

$(OUTPUT)\Notepad2.obj : \
  $(SRC)\Notepad2.c \
  $(SCI)\include\scintilla.h \
  $(SCI)\include\scilexer.h \
  $(SRC)\Edit.h \
  $(SRC)\Styles.h \
  $(SRC)\Helpers.h \
  $(SRC)\Dialogs.h \
  $(SRC)\Notepad2.h \
  $(SRC)\resource.h

$(OUTPUT)\Styles.obj : \
  $(SRC)\Styles.c \
  $(SRC)\Dialogs.h \
  $(SRC)\Helpers.h \
  $(SRC)\Notepad2.h \
  $(SCI)\include\scintilla.h \
  $(SCI)\include\scilexer.h \
  $(SRC)\Edit.h \
  $(SRC)\Styles.h \
  $(SRC)\resource.h

$(OUTPUT)\Print.obj : \
  $(SRC)\Print.cpp \
  $(SCI)\include\platform.h \
  $(SCI)\include\scintilla.h \
  $(SCI)\include\scilexer.h \
  $(SRC)\Dialogs.h \
  $(SRC)\Helpers.h \
  $(SRC)\resource.h

$(SRC)\Notepad2.rc : \
  $(SRC)\resource.h \
  $(SRC)\version.h \
  $(SRC)\Notepad2.ver \
  $(RES)\Encoding.bmp \
  $(RES)\Next.bmp \
  $(RES)\Open.bmp \
  $(RES)\Pick.bmp \
  $(RES)\Prev.bmp \
  $(RES)\Toolbar.bmp \
  $(RES)\Copy.cur \
  $(RES)\Notepad2.ico \
  $(RES)\Run.ico \
  $(RES)\Styles.ico \
  $(RES)\Notepad2.exe.manifest
