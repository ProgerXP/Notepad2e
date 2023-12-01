#include "Lexers.h"


KEYWORDLIST KeyWords_NULL = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexDefault = { SCLEX_NULL, 63000, L"Default Text", L"txt; text; wtx; log; asc; doc; diz; nfo", L"", &KeyWords_NULL, NULL_COMMENT_INFO, {
  // [2e]: NOTE: update EDefaultLexerOptions enum in case of changed order
  /*  0 */ { STYLE_DEFAULT, 63100, L"Default Style", L"font:Lucida Console; size:10", L"" },
  /*  1 */ { STYLE_LINENUMBER, 63101, L"Margins and Line Numbers", L"size:-1", L"" },
  /*  2 */ { STYLE_BRACELIGHT, 63102, L"Matching Braces", L"size:+1; bold; fore:#FF0000", L"" },
  /*  3 */ { STYLE_BRACEBAD, 63103, L"Matching Braces Error", L"size:+1; bold; fore:#000080", L"" },
  /*  4 */ { STYLE_CONTROLCHAR, 63104, L"Control Characters (Font)", L"size:-1", L"" },
  /*  5 */ { STYLE_INDENTGUIDE, 63105, L"Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
  /*  6 */ { SCI_SETSELFORE + SCI_SETSELBACK, 63106, L"Selected Text (Colors)", L"back:#0A246A; eolfilled; alpha:95", L"" },
  /*  7 */ { SCI_SETWHITESPACEFORE + SCI_SETWHITESPACEBACK + SCI_SETWHITESPACESIZE, 63107, L"Whitespace (Colors, Size 0-5)", L"fore:#FF4000", L"" },
  /*  8 */ { SCI_SETCARETLINEBACK + SCI_SETCARETLINEBACKALPHA, 63108, L"Current Line Background (Color)", L"back:#FFFF00; alpha:50", L"" },
  /*  9 */ { SCI_SETCARETLINEBACK + SCI_SETCARETLINEBACKALPHA, 63361, L"Current Line, Inactive View (Color)", L"back:#999999; alpha:50", L"" },
  /* 10 */ { SCI_SETCARETFORE + SCI_SETCARETWIDTH, 63109, L"Caret (Color, Size 1-3)", L"", L"" },
  /* 11 */ { SCI_SETEDGECOLOUR, 63110, L"Long Line Marker (Colors)", L"fore:#FFC000", L"" },
  /* 12 */ { STYLE_LINEINDICATOR, 63363, L"Find Marker (Color)", L"fore:#FFCCCC", L"" },
  /* 13 */ { STYLE_LINEINDICATOR_FIRST_LAST, 63365, L"Find Marker First/Last (Color)", L"fore:#FF0000", L"" },
  /* 14 */ { SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT, 63111, L"Extra Line Spacing (Size)", L"size:2", L"" },
  /* 15 */ { 0, 63124, L"Splitter (Color, Size)", L"size:0", L"" },

  /* 16 */ { STYLE_DEFAULT, 63112, L"2nd Default Style", L"font:Courier New; size:10", L"" },
  /* 17 */ { STYLE_LINENUMBER, 63113, L"2nd Margins and Line Numbers", L"font:Tahoma; size:-2; fore:#FF0000", L"" },
  /* 18 */ { STYLE_BRACELIGHT, 63114, L"2nd Matching Braces", L"bold; fore:#FF0000", L"" },
  /* 19 */ { STYLE_BRACEBAD, 63115, L"2nd Matching Braces Error", L"bold; fore:#000080", L"" },
  /* 20 */ { STYLE_CONTROLCHAR, 63116, L"2nd Control Characters (Font)", L"size:-1", L"" },
  /* 21 */ { STYLE_INDENTGUIDE, 63117, L"2nd Indentation Guide (Color)", L"fore:#A0A0A0", L"" },
  /* 22 */ { SCI_SETSELFORE + SCI_SETSELBACK, 63118, L"2nd Selected Text (Colors)", L"eolfilled", L"" },
  /* 23 */ { SCI_SETWHITESPACEFORE + SCI_SETWHITESPACEBACK + SCI_SETWHITESPACESIZE, 63119, L"2nd Whitespace (Colors, Size 0-5)", L"fore:#FF4000", L"" },
  /* 24 */ { SCI_SETCARETLINEBACK + SCI_SETCARETLINEBACKALPHA, 63120, L"2nd Current Line Background (Color)", L"back:#FFFF00; alpha:50", L"" },
  /* 25 */ { SCI_SETCARETLINEBACK + SCI_SETCARETLINEBACKALPHA, 63362, L"2nd Current Line, Inactive View (Color)", L"back:#999999; alpha:50", L"" },
  /* 26 */ { SCI_SETCARETFORE + SCI_SETCARETWIDTH, 63121, L"2nd Caret (Color, Size 1-3)", L"", L"" },
  /* 27 */ { SCI_SETEDGECOLOUR, 63122, L"2nd Long Line Marker (Colors)", L"fore:#FFC000", L"" },
  /* 28 */ { STYLE_LINEINDICATOR, 63364, L"2nd Find Marker (Color)", L"fore:#FF2D2D", L"" },
  /* 29 */ { STYLE_LINEINDICATOR_FIRST_LAST, 63366, L"2nd Find Marker First/Last (Color)", L"fore:#00FF00", L"" },
  /* 30 */ { SCI_SETEXTRAASCENT + SCI_SETEXTRADESCENT, 63123, L"2nd Extra Line Spacing (Size)", L"", L"" },
  { -1, 00000, L"", L"", L"" }
}
};


KEYWORDLIST KeyWords_HTML = {
    "!doctype ^aria- ^data- a abbr accept accept-charset accesskey acronym action address align alink "
    "alt and applet archive area article aside async audio autocomplete autofocus autoplay axis b "
    "background base basefont bb bdi bdo bgcolor big blockquote body border bordercolor br button "
    "canvas caption cellpadding cellspacing center challenge char charoff charset checkbox checked "
    "cite class classid clear code codebase codetype col colgroup color cols colspan command compact "
    "content contenteditable contextmenu controls coords data datafld dataformatas datagrid datalist "
    "datapagesize datasrc datetime dd declare default defer del details dfn dialog dir dirname "
    "disabled div dl draggable dropzone dt em embed enctype event eventsource face fieldset "
    "figcaption figure file font footer for form formaction formenctype formmethod formnovalidate "
    "formtarget frame frameborder frameset h1 h2 h3 h4 h5 h6 head header headers height hgroup hidden "
    "high hr href hreflang hspace html http-equiv i icon id iframe image img input ins isindex ismap "
    "kbd keygen keytype kind label lang language leftmargin legend li link list longdesc loop low "
    "main manifest map marginheight marginwidth mark max maxlength media menu meta meter method min "
    "multiple name nav noframes nohref noresize noscript noshade novalidate nowrap object ol onabort "
    "onafterprint onbeforeprint onbeforeunload onblur oncanplay oncanplaythrough onchange onclick "
    "oncontextmenu oncuechange ondblclick ondrag ondragend ondragenter ondragleave ondragover "
    "ondragstart ondrop ondurationchange onemptied onended onerror onfocus onformchange onforminput "
    "onhashchange oninput oninvalid onkeydown onkeypress onkeyup onload onloadeddata onloadedmetadata "
    "onloadstart onmessage onmousedown onmousemove onmouseout onmouseover onmouseup onmousewheel "
    "onoffline ononline onpagehide onpageshow onpause onplay onplaying onpopstate onprogress "
    "onratechange onreadystatechange onredo onreset onresize onscroll onseeked onseeking onselect "
    "onshow onstalled onstorage onsubmit onsuspend ontimeupdate onundo onunload onvolumechange "
    "onwaiting open optgroup optimum option output p param password pattern picture ping placeholder poster "
    "pre preload profile progress prompt pubdate public q radio radiogroup rb readonly rel required "
    "reset rev reversed rows rowspan rp rt rtc ruby rules s samp sandbox scheme scope scoped script "
    "scrolling seamless section select selected shape size sizes small source span spellcheck src "
    "srcdoc srclang standby start step strike strong style sub submit summary sup tabindex table "
    "target tbody td template text textarea tfoot th thead time title topmargin tr track tt type u ul usemap "
    "valign value valuetype var version video vlink vspace wbr width wrap xml xmlns",
    "abstract boolean break byte case catch char class const continue debugger default delete do "
    "double else enum export extends false final finally float for function goto if implements "
    "import in instanceof int interface long native new null package private protected public "
    "return short static super switch synchronized this throw throws transient true try typeof var "
    "void volatile while with",
    "alias and as attribute begin boolean byref byte byval call case class compare const continue "
    "currency date declare dim do double each else elseif empty end enum eqv erase error event exit "
    "explicit false for friend function get global gosub goto if imp implement in integer is let lib "
    "load long loop lset me mid mod module new next not nothing null object on option optional or "
    "preserve private property public raiseevent redim rem resume return rset select set single "
    "static stop string sub then to true type unload until variant wend while with withevents xor",
    "",
    "__callstatic __class__ __dir__ __file__ __function__ __get __isset __line__ __method__ "
    "__namespace__ __set __sleep __unset __wakeup abstract and argc argv array as break case catch "
    "cfunction class clone const continue declare default define die do e_all e_error e_fatal "
    "e_notice e_parse e_strict e_warning echo else elseif empty enddeclare endfor endforeach endif "
    "endswitch endwhile eval exception exit extends false final for foreach function global goto "
    "http_cookie_vars http_env_vars http_get_vars http_post_files http_post_vars http_server_vars if "
    "implements include include_once instanceof interface isset list namespace new not null "
    "old_function or parent php_self print private protected public require require_once return "
    "static stdclass switch this throw true try unset use var virtual while xor",
    "", "", "", ""
};


EDITLEXER lexHTML = { SCLEX_HTML, 63001, L"Web Source Code", L"html; htm; asp; aspx; shtml; htd; xhtml; php; php3; phtml; htt; cfm; tpl; dtd; hta; htc", L"", &KeyWords_HTML, COMMENT_INFO(C_COMMENT, MULTI_STYLE(SCE_HPHP_COMMENTLINE, SCE_HJ_COMMENTLINE, SCE_HJA_COMMENTLINE, SCE_HBA_COMMENTLINE)), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_H_TAG, SCE_H_TAGEND, 0, 0), 63136, L"HTML Tag", L"fore:#648000", L"" },
        { SCE_H_TAGUNKNOWN, 63137, L"HTML Unknown Tag", L"fore:#C80000; back:#FFFF80", L"" },
        { SCE_H_ATTRIBUTE, 63138, L"HTML Attribute", L"fore:#FF4000", L"" },
        { SCE_H_ATTRIBUTEUNKNOWN, 63139, L"HTML Unknown Attribute", L"fore:#C80000; back:#FFFF80", L"" },
        { SCE_H_VALUE, 63140, L"HTML Value", L"fore:#3A6EA5", L"" },
        { MULTI_STYLE(SCE_H_DOUBLESTRING, SCE_H_SINGLESTRING, 0, 0), 63141, L"HTML String", L"fore:#3A6EA5", L"" },
        { SCE_H_OTHER, 63142, L"HTML Other Inside Tag", L"fore:#3A6EA5", L"" },
        { MULTI_STYLE(SCE_H_COMMENT, SCE_H_XCCOMMENT, 0, 0), 63143, L"HTML Comment", L"fore:#646464", L"" },
        { SCE_H_ENTITY, 63144, L"HTML Entity", L"fore:#B000B0", L"" },
        { SCE_H_DEFAULT, 63256, L"HTML Element Text", L"", L"" },
        { MULTI_STYLE(SCE_H_XMLSTART, SCE_H_XMLEND, 0, 0), 63145, L"XML Identifier", L"bold; fore:#881280", L"" },
        { SCE_H_SGML_DEFAULT, 63237, L"SGML", L"fore:#881280", L"" },
        { SCE_H_CDATA, 63147, L"CDATA", L"fore:#646464", L"" },
        { MULTI_STYLE(SCE_H_ASP, SCE_H_ASPAT, 0, 0), 63146, L"ASP Start Tag", L"bold; fore:#000080", L"" },
        { SCE_H_QUESTION, 63148, L"PHP Start Tag", L"bold; fore:#000080", L"" },
        { SCE_HPHP_DEFAULT, 63149, L"PHP Default", L"", L"" },
        { MULTI_STYLE(SCE_HPHP_COMMENT, SCE_HPHP_COMMENTLINE, 0, 0), 63157, L"PHP Comment", L"fore:#FF8000", L"" },
        { SCE_HPHP_WORD, 63152, L"PHP Keyword", L"bold; fore:#A46000", L"" },
        { SCE_HPHP_HSTRING, 63150, L"PHP String", L"fore:#008000", L"" },
        { SCE_HPHP_SIMPLESTRING, 63151, L"PHP Simple String", L"fore:#008000", L"" },
        { SCE_HPHP_NUMBER, 63153, L"PHP Number", L"fore:#FF0000", L"" },
        { SCE_HPHP_OPERATOR, 63158, L"PHP Operator", L"fore:#B000B0", L"" },
        { SCE_HPHP_VARIABLE, 63154, L"PHP Variable", L"italic; fore:#000080", L"" },
        { SCE_HPHP_HSTRING_VARIABLE, 63155, L"PHP String Variable", L"italic; fore:#000080", L"" },
        { SCE_HPHP_COMPLEX_VARIABLE, 63156, L"PHP Complex Variable", L"italic; fore:#000080", L"" },
        { MULTI_STYLE(SCE_HJ_DEFAULT, SCE_HJ_START, 0, 0), 63159, L"JS Default", L"", L"" },
        { MULTI_STYLE(SCE_HJ_COMMENT, SCE_HJ_COMMENTLINE, SCE_HJ_COMMENTDOC, 0), 63160, L"JS Comment", L"fore:#646464", L"" },
        { SCE_HJ_KEYWORD, 63163, L"JS Keyword", L"bold; fore:#A46000", L"" },
        { SCE_HJ_WORD, 63162, L"JS Identifier", L"", L"" },
        { MULTI_STYLE(SCE_HJ_DOUBLESTRING, SCE_HJ_SINGLESTRING, SCE_HJ_STRINGEOL, 0), 63164, L"JS String", L"fore:#008000", L"" },
        { SCE_HJ_REGEX, 63166, L"JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_HJ_NUMBER, 63161, L"JS Number", L"fore:#FF0000", L"" },
        { SCE_HJ_SYMBOLS, 63165, L"JS Symbols", L"fore:#B000B0", L"" },
        { MULTI_STYLE(SCE_HJA_DEFAULT, SCE_HJA_START, 0, 0), 63167, L"ASP JS Default", L"", L"" },
        { MULTI_STYLE(SCE_HJA_COMMENT, SCE_HJA_COMMENTLINE, SCE_HJA_COMMENTDOC, 0), 63168, L"ASP JS Comment", L"fore:#646464", L"" },
        { SCE_HJA_KEYWORD, 63171, L"ASP JS Keyword", L"bold; fore:#A46000", L"" },
        { SCE_HJA_WORD, 63170, L"ASP JS Identifier", L"", L"" },
        { MULTI_STYLE(SCE_HJA_DOUBLESTRING, SCE_HJA_SINGLESTRING, SCE_HJA_STRINGEOL, 0), 63172, L"ASP JS String", L"fore:#008000", L"" },
        { SCE_HJA_REGEX, 63174, L"ASP JS Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_HJA_NUMBER, 63169, L"ASP JS Number", L"fore:#FF0000", L"" },
        { SCE_HJA_SYMBOLS, 63173, L"ASP JS Symbols", L"fore:#B000B0", L"" },
        { MULTI_STYLE(SCE_HB_DEFAULT, SCE_HB_START, 0, 0), 63175, L"VBS Default", L"", L"" },
        { SCE_HB_COMMENTLINE, 63176, L"VBS Comment", L"fore:#646464", L"" },
        { SCE_HB_WORD, 63178, L"VBS Keyword", L"bold; fore:#B000B0", L"" },
        { SCE_HB_IDENTIFIER, 63180, L"VBS Identifier", L"", L"" },
        { MULTI_STYLE(SCE_HB_STRING, SCE_HB_STRINGEOL, 0, 0), 63179, L"VBS String", L"fore:#008000", L"" },
        { SCE_HB_NUMBER, 63177, L"VBS Number", L"fore:#FF0000", L"" },
        { MULTI_STYLE(SCE_HBA_DEFAULT, SCE_HBA_START, 0, 0), 63181, L"ASP VBS Default", L"", L"" },
        { SCE_HBA_COMMENTLINE, 63182, L"ASP VBS Comment", L"fore:#646464", L"" },
        { SCE_HBA_WORD, 63184, L"ASP VBS Keyword", L"bold; fore:#B000B0", L"" },
        { SCE_HBA_IDENTIFIER, 63186, L"ASP VBS Identifier", L"", L"" },
        { MULTI_STYLE(SCE_HBA_STRING, SCE_HBA_STRINGEOL, 0, 0), 63185, L"ASP VBS String", L"fore:#008000", L"" },
        { SCE_HBA_NUMBER, 63183, L"ASP VBS Number", L"fore:#FF0000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_XML = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexXML = { SCLEX_XML, 63002, L"XML", L"xml; xsl; rss; svg; xul; xsd; xslt; axl; rdf; xaml; vcproj", L"", &KeyWords_XML, COMMENT_INFO(C_COMMENT, 0), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_H_TAG, SCE_H_TAGUNKNOWN, SCE_H_TAGEND, 0), 63187, L"XML Tag", L"fore:#881280", L"" },
        { MULTI_STYLE(SCE_H_ATTRIBUTE, SCE_H_ATTRIBUTEUNKNOWN, 0, 0), 63188, L"XML Attribute", L"fore:#994500", L"" },
        { SCE_H_VALUE, 63189, L"XML Value", L"fore:#1A1AA6", L"" },
        { MULTI_STYLE(SCE_H_DOUBLESTRING, SCE_H_SINGLESTRING, 0, 0), 63190, L"XML String", L"fore:#1A1AA6", L"" },
        { SCE_H_OTHER, 63191, L"XML Other Inside Tag", L"fore:#1A1AA6", L"" },
        { MULTI_STYLE(SCE_H_COMMENT, SCE_H_XCCOMMENT, 0, 0), 63192, L"XML Comment", L"fore:#646464", L"" },
        { SCE_H_ENTITY, 63193, L"XML Entity", L"fore:#B000B0", L"" },
        { SCE_H_DEFAULT, 63257, L"XML Element Text", L"", L"" },
        { MULTI_STYLE(SCE_H_XMLSTART, SCE_H_XMLEND, 0, 0), 63145, L"XML Identifier", L"bold; fore:#881280", L"" },
        { SCE_H_SGML_DEFAULT, 63237, L"SGML", L"fore:#881280", L"" },
        { SCE_H_CDATA, 63147, L"CDATA", L"fore:#646464", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_CSS = {
    "^-moz- ^-ms- ^-o- ^-webkit-"
    " animation animation-name animation-duration animation-timing-function animation-delay animation-iteration-count animation-direction animation-play-state"
    " background background-attachment background-color background-image background-position background-repeat background-clip background-origin background-size"
    " border border-bottom border-bottom-color border-bottom-style border-bottom-width border-color border-left border-left-color border-left-style border-left-width"
    " border-right border-right-color border-right-style border-right-width border-style border-top border-top-color border-top-style border-top-width border-width"
    " outline outline-color outline-style outline-width border-bottom-left-radius border-bottom-right-radius"
    " border-image border-image-outset border-image-repeat border-image-slice border-image-source border-image-width"
    " border-radius border-top-left-radius border-top-right-radius box-decoration-break box-shadow"
    " overflow-x overflow-y overflow-style rotation rotation-point overflow-x overflow-y overflow-style"
    " rotation rotation-point bookmark-label bookmark-level bookmark-target float-offset hyphenate-after hyphenate-before hyphenate-character hyphenate-lines hyphenate-resource"
    " hyphens image-resolution marks string-set height max-height max-width min-height min-width width"
    " box-align box-direction box-flex box-flex-group box-lines box-ordinal-group box-orient box-pack font font-family font-size font-style"
    " font-variant font-weight font-size-adjust font-stretch content counter-increment counter-reset quotes crop move-to page-policy grid-columns grid-rows"
    " target target-name target-new target-position alignment-adjust alignment-baseline baseline-shift dominant-baseline drop-initial-after-adjust"
    " drop-initial-after-align drop-initial-before-adjust drop-initial-before-align drop-initial-size drop-initial-value inline-box-align line-stacking line-stacking-ruby"
    " line-stacking-shift line-stacking-strategy text-height list-style list-style-image list-style-position list-style-type"
    " margin margin-bottom margin-left margin-right margin-top marquee-direction marquee-play-count marquee-speed marquee-style"
    " column-count column-fill column-gap column-rule column-rule-color column-rule-style column-rule-width column-span column-width"
    " columns padding padding-bottom padding-left padding-right padding-top fit fit-position image-orientation"
    " page size bottom clear clip cursor display float left overflow position right top visibility"
    " z-index orphans page-break-after page-break-before page-break-inside widows ruby-align ruby-overhang ruby-position ruby-span mark mark-after mark-before"
    " phonemes rest rest-after rest-before voice-balance voice-duration voice-pitch voice-pitch-range voice-rate voice-stress voice-volume border-collapse border-spacing caption-side"
    " empty-cells table-layout color direction letter-spacing line-height text-align text-decoration text-indent text-transform unicode-bidi vertical-align white-space"
    " word-spacing hanging-punctuation punctuation-trim text-align-last text-justify text-outline text-overflow text-shadow text-wrap"
    " word-break word-wrap transform transform-origin transform-style perspective perspective-origin backface-visibility"
    " transition transition-property transition-duration transition-timing-function transition-delay appearance box-sizing icon"
    " nav-down nav-index nav-left nav-right nav-up outline-offset resize color-profile opacity rendering-intent"
    ,
    "active after before checked default disabled empty enabled first first-child first-letter"
    " first-line first-of-type focus hover indeterminate invalid lang last-child last-of-type left"
    " link not nth-child nth-last-child nth-last-of-type nth-of-type only-child only-of-type optional"
    " required right root target valid visited"
    ,
    "", "", "", "", "", "", ""
};


EDITLEXER lexCSS = { SCLEX_CSS, 63003, L"CSS Style Sheets", L"css", L"", &KeyWords_CSS, COMMENT_INFO(C_COMMENT, 0), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_CSS_COMMENT, 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_CSS_TAG, 63136, L"HTML Tag", L"bold; fore:#0A246A", L"" },
        { SCE_CSS_CLASS, 63194, L"Tag-Class", L"fore:#648000", L"" },
        { SCE_CSS_ID, 63195, L"Tag-Id", L"fore:#648000", L"" },
        { SCE_CSS_ATTRIBUTE, 63196, L"Tag-Attribute", L"italic; fore:#648000", L"" },
        { MULTI_STYLE(SCE_CSS_PSEUDOCLASS, SCE_CSS_EXTENDED_PSEUDOCLASS, SCE_CSS_PSEUDOELEMENT, SCE_CSS_EXTENDED_PSEUDOELEMENT), 63197, L"Pseudo-class/element", L"fore:#B000B0", L"" },
        { SCE_CSS_UNKNOWN_PSEUDOCLASS, 63198, L"Unknown Pseudo-class", L"fore:#C80000; back:#FFFF80", L"" },
        { MULTI_STYLE(SCE_CSS_IDENTIFIER, SCE_CSS_IDENTIFIER2, SCE_CSS_IDENTIFIER3, SCE_CSS_EXTENDED_IDENTIFIER), 63199, L"CSS Property", L"fore:#FF4000", L"" },
        { SCE_CSS_UNKNOWN_IDENTIFIER, 63200, L"Unknown Property", L"fore:#C80000; back:#FFFF80", L"" },
        { MULTI_STYLE(SCE_CSS_DOUBLESTRING, SCE_CSS_SINGLESTRING, 0, 0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_CSS_VALUE, 63201, L"Value", L"fore:#3A6EA5", L"" },
        { SCE_CSS_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { SCE_CSS_IMPORTANT, 63202, L"Important", L"bold; fore:#C80000", L"" },
        { SCE_CSS_DIRECTIVE, 63203, L"Directive", L"bold; fore:#000000; back:#FFF1A8", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_CPP = {
    "__abstract __alignof __asm __assume __based __box __cdecl __declspec __delegate __event "
    "__except __except__try __fastcall __finally __forceinline __gc __hook __identifier "
    "__if_exists __if_not_exists __inline __int16 __int32 __int64 __int8 __interface __leave "
    "__m128 __m128d __m128i __m64 __multiple_inheritance __nogc __noop __pin __property __raise "
    "__sealed __single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value "
    "__virtual_inheritance __wchar_t auto bool break case catch char class const const_cast "
    "continue default defined delete do double dynamic_cast else enum explicit extern false float "
    "for friend goto if inline int long mutable naked namespace new operator private protected "
    "public register reinterpret_cast return short signed size_t sizeof static static_cast struct "
    "switch template this throw true try typedef typeid typename union unsigned using uuid "
    "virtual void volatile wchar_t while",
    "",
    "", "", "", "", "", "", ""
};


EDITLEXER lexCPP = { SCLEX_CPP, 63004, L"C/C++", L"c; cpp; cxx; cc; h; hpp; hxx; hh; m; mm; idl; inl; odl", L"", &KeyWords_CPP, COMMENT_INFO(C_COMMENT, SCE_C_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_C_STRING, SCE_C_CHARACTER, SCE_C_STRINGEOL, SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
        { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_CS = {
    "abstract as base bool break byte case catch char checked class const "
    "continue decimal default delegate do double else enum event explicit "
    "extern false finally fixed float for foreach goto if implicit in int interface "
    "internal is lock long namespace new null object operator out override "
    "params private protected public readonly ref return sbyte sealed short "
    "sizeof stackalloc static string struct switch this throw true try typeof "
    "uint ulong unchecked unsafe ushort using virtual void while",
    "",
    "", "", "", "", "", "", ""
};


EDITLEXER lexCS = { SCLEX_CPP, 63005, L"C#", L"cs", L"", &KeyWords_CS, COMMENT_INFO(C_COMMENT, SCE_C_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
        { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_C_STRING, SCE_C_CHARACTER, SCE_C_STRINGEOL, 0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_C_VERBATIM, 63134, L"Verbatim String", L"fore:#008000", L"" },
        { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_RC = {
    "ACCELERATORS ALT AUTO3STATE AUTOCHECKBOX AUTORADIOBUTTON "
    "BEGIN BITMAP BLOCK BUTTON CAPTION CHARACTERISTICS CHECKBOX "
    "CLASS COMBOBOX CONTROL CTEXT CURSOR DEFPUSHBUTTON DIALOG "
    "DIALOGEX DISCARDABLE EDITTEXT END EXSTYLE FONT GROUPBOX "
    "ICON LANGUAGE LISTBOX LTEXT MENU MENUEX MENUITEM "
    "MESSAGETABLE POPUP PUSHBUTTON RADIOBUTTON RCDATA RTEXT "
    "SCROLLBAR SEPARATOR SHIFT STATE3 STRINGTABLE STYLE "
    "TEXTINCLUDE VALUE VERSION VERSIONINFO VIRTKEY",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexRC = { SCLEX_CPP, 63006, L"Resource Script", L"rc; rc2; rct; rh; r; dlg", L"", &KeyWords_RC, COMMENT_INFO(C_COMMENT, SCE_C_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_C_STRING, SCE_C_CHARACTER, SCE_C_STRINGEOL, SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
        { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#0A246A", L"" },
        { SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_MAK = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexMAK = { SCLEX_MAKEFILE, 63007, L"Makefile", L"mak; make; mk; dsp", L"", &KeyWords_MAK, COMMENT_INFO(BASH_COMMENT, SCE_MAKE_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"fore:#0A246A", L"" },
        { SCE_MAKE_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
        { MULTI_STYLE(SCE_MAKE_IDENTIFIER, SCE_MAKE_IDEOL, 0, 0), 63129, L"Identifier", L"fore:#003CE6", L"" },
        { SCE_MAKE_OPERATOR, 63132, L"Operator", L"", L"" },
        { SCE_MAKE_TARGET, 63204, L"Target", L"fore:#003CE6; back:#FFC000", L"" },
        { SCE_MAKE_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_VBS = {
    "alias and as attribute begin boolean byref byte byval call case class compare const continue "
    "currency date declare dim do double each else elseif empty end enum eqv erase error event exit "
    "explicit false for friend function get global gosub goto if imp implement in integer is let lib "
    "load long loop lset me mid mod module new next not nothing null object on option optional or "
    "preserve private property public raiseevent redim rem resume return rset select set single "
    "static stop string sub then to true type unload until variant wend while with withevents xor",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexVBS = { SCLEX_VBSCRIPT, 63008, L"VBScript", L"vbs; dsm", L"", &KeyWords_VBS, COMMENT_INFO(VB_COMMENT, SCE_B_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_B_COMMENT, SCE_B_COMMENTBLOCK, 0, 0), 63127, L"Comment", L"fore:#808080", L"" },
        { SCE_B_KEYWORD, 63128, L"Keyword", L"bold; fore:#B000B0", L"" },
        { SCE_B_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_B_STRING, SCE_B_STRINGEOL, 0, 0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_B_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_B_OPERATOR, 63132, L"Operator", L"", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_VB = {
    "addhandler addressof alias and andalso ansi any as assembly auto boolean byref byte byval call "
    "case catch cbool cbyte cchar cdate cdbl cdec char cint class clng cobj compare const cshort csng "
    "cstr ctype date decimal declare default delegate dim directcast do double each else elseif end "
    "enum erase error event exit explicit externalsource false finally for friend function get "
    "gettype gosub goto handles if implements imports in inherits integer interface is let lib like "
    "long loop me mid mod module mustinherit mustoverride mybase myclass namespace new next not "
    "nothing notinheritable notoverridable object on option optional or orelse overloads overridable "
    "overrides paramarray preserve private property protected public raiseevent randomize readonly "
    "redim rem removehandler resume return select set shadows shared short single static step stop "
    "strict string structure sub synclock then throw to true try typeof unicode until variant when "
    "while with withevents writeonly xor",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexVB = { SCLEX_VB, 63009, L"Visual Basic", L"vb; bas; frm; cls; ctl; pag; dsr; dob", L"", &KeyWords_VB, COMMENT_INFO(VB_COMMENT, SCE_B_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_B_COMMENT, SCE_B_COMMENTBLOCK, 0, 0), 63127, L"Comment", L"fore:#808080", L"" },
        { SCE_B_KEYWORD, 63128, L"Keyword", L"bold; fore:#B000B0", L"" },
        { SCE_B_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_B_STRING, SCE_B_STRINGEOL, 0, 0), 63131, L"String", L"fore:#008000", L"" },
        { MULTI_STYLE(SCE_B_NUMBER, SCE_B_DATE, 0, 0), 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_B_OPERATOR, 63132, L"Operator", L"", L"" },
        { SCE_B_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF9C00", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_JS = {
    "abstract boolean break byte case catch char class const continue debugger default delete do "
    "double else enum export extends false final finally float for function goto if implements "
    "import in instanceof int interface long native new null package private protected public "
    "return short static super switch synchronized this throw throws transient true try typeof var "
    "void volatile while with",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexJS = { SCLEX_CPP, 63010, L"JavaScript", L"js; jse; jsm; json; as", L"", &KeyWords_JS, COMMENT_INFO(C_COMMENT, SCE_C_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#A46000", L"" },
        { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_C_STRING, SCE_C_CHARACTER, SCE_C_STRINGEOL, SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
        { SCE_C_REGEX, 63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_JAVA = {
    "abstract assert boolean break byte case catch char class const "
    "continue default do double else extends final finally float for future "
    "generic goto if implements import inner instanceof int interface long "
    "native new null outer package private protected public rest return "
    "short static super switch synchronized this throw throws transient try "
    "var void volatile while",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexJAVA = { SCLEX_CPP, 63011, L"Java", L"java", L"", &KeyWords_JAVA, COMMENT_INFO(C_COMMENT, SCE_C_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_C_COMMENT, SCE_C_COMMENTLINE, SCE_C_COMMENTDOC, SCE_C_COMMENTLINEDOC, SCE_C_COMMENTDOCKEYWORD, SCE_C_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#A46000", L"" },
        { SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_C_STRING, SCE_C_CHARACTER, SCE_C_STRINGEOL, SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
        { SCE_C_REGEX, 63135, L"Regex", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_PAS = {
    "absolute abstract alias and array as asm assembler begin break case cdecl class const constructor continue cppdecl default "
    "destructor dispose div do downto else end end. except exit export exports external false far far16 file finalization finally for "
    "forward function goto if implementation in index inherited initialization inline interface is label library local message mod "
    "name near new nil nostackframe not object of oldfpccall on operator or out overload override packed pascal private procedure "
    "program property protected public published raise read record register reintroduce repeat resourcestring safecall self set shl "
    "shr softfloat stdcall stored string then threadvar to true try type unit until uses var virtual while with write xor",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexPAS = { SCLEX_PASCAL, 63012, L"Pascal/Delphi", L"pas; dpr; dpk; dfm; inc; pp", L"", &KeyWords_PAS, COMMENT_INFO(PASCAL_COMMENT, SCE_PAS_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_PAS_COMMENT, SCE_PAS_COMMENT2, SCE_PAS_COMMENTLINE, 0), 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_PAS_WORD, 63128, L"Keyword", L"bold; fore:#800080", L"" },
        { SCE_PAS_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_PAS_STRING, SCE_PAS_CHARACTER, SCE_PAS_STRINGEOL, 0), 63131, L"String", L"fore:#008000", L"" },
        { MULTI_STYLE(SCE_PAS_NUMBER, SCE_PAS_HEXNUMBER, 0, 0), 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_PAS_OPERATOR, 63132, L"Operator", L"bold", L"" },
        { SCE_PAS_ASM, 63205, L"Inline Asm", L"fore:#0000FF", L"" },
        { MULTI_STYLE(SCE_PAS_PREPROCESSOR, SCE_PAS_PREPROCESSOR2, 0, 0), 63133, L"Preprocessor", L"fore:#FF00FF", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_ASM = {
    "aaa aad aam aas adc add and arpl bound bsf bsr bswap bt btc btr bts call cbw cdq cflush clc cld "
    "cli clts cmc cmova cmovae cmovb cmovbe cmovc cmove cmovg cmovge cmovl cmovle cmovna cmovnae "
    "cmovnb cmovnbe cmovnc cmovne cmovng cmovnge cmovnl cmovnle cmovno cmovnp cmovns cmovnz cmovo "
    "cmovp cmovpe cmovpo cmovs cmovz cmp cmps cmpsb cmpsd cmpsq cmpsw cmpxchg cmpxchg486 cmpxchg8b "
    "cpuid cwd cwde daa das dec div emms enter esc femms hlt ibts icebp idiv imul in inc ins insb "
    "insd insw int int01 int03 int1 int3 into invd invlpg iret iretd iretdf iretf iretw ja jae jb jbe "
    "jc jcxz je jecxz jg jge jl jle jmp jna jnae jnb jnbe jnc jne jng jnge jnl jnle jno jnp jns jnz "
    "jo jp jpe jpo js jz lahf lar lds lea leave les lfs lgdt lgs lidt lldt lmsw loadall loadall286 "
    "lock lods lodsb lodsd lodsq lodsw loop loopd loope looped loopew loopne loopned loopnew loopnz "
    "loopnzd loopnzw loopw loopz loopzd loopzw lsl lss ltr mov movs movsb movsd movsq movsw movsx "
    "movsxd movzx mul neg nop not or out outs outsb outsd outsw pop popa popad popaw popf popfd popfw "
    "push pusha pushad pushaw pushd pushf pushfd pushfw pushw rcl rcr rdmsr rdpmc rdshr rdtsc rep "
    "repe repne repnz repz ret retf retn rol ror rsdc rsldt rsm rsts sahf sal salc sar sbb scas scasb "
    "scasd scasq scasw seta setae setb setbe setc sete setg setge setl setle setna setnae setnb "
    "setnbe setnc setne setng setnge setnl setnle setno setnp setns setnz seto setp setpe setpo sets "
    "setz sgdt shl shld shr shrd sidt sldt smi smint smintold smsw stc std sti stos stosb stosd stosq "
    "stosw str sub svdc svldt svts syscall sysenter sysexit sysret test ud0 ud1 ud2 umov verr verw "
    "wait wbinvd wrmsr wrshr xadd xbts xchg xlat xlatb xor",
    "f2xm1 fabs fadd faddp fbld fbstp fchs fclex fcmovb fcmovbe fcmove fcmovnb fcmovnbe fcmovne "
    "fcmovnu fcmovu fcom fcomi fcomip fcomp fcompp fcos fdecstp fdisi fdiv fdivp fdivr fdivrp feni "
    "ffree ffreep fiadd ficom ficomp fidiv fidivr fild fimul fincstp finit fist fistp fisub fisubr "
    "fld fld1 fldcw fldenv fldenvd fldenvw fldl2e fldl2t fldlg2 fldln2 fldpi fldz fmul fmulp fnclex "
    "fndisi fneni fninit fnop fnsave fnsaved fnsavew fnstcw fnstenv fnstenvd fnstenvw fnstsw fpatan "
    "fprem fprem1 fptan frndint frstor frstord frstorw fsave fsaved fsavew fscale fsetpm fsin fsincos "
    "fsqrt fst fstcw fstenv fstenvd fstenvw fstp fstsw fsub fsubp fsubr fsubrp ftst fucom fucomp "
    "fucompp fwait fxam fxch fxtract fyl2x fyl2xp1",
    "ah al ax bh bl bp bx ch cl cr0 cr2 cr3 cr4 cs cx dh di dl dr0 dr1 dr2 dr3 dr6 dr7 ds dx eax ebp "
    "ebx ecx edi edx eip es esi esp fs gs mm0 mm1 mm2 mm3 mm4 mm5 mm6 mm7 r10 r10b r10d r10w r11 r11b "
    "r11d r11w r12 r12b r12d r12w r13 r13b r13d r13w r14 r14b r14d r14w r15 r15b r15d r15w r8 r8b r8d "
    "r8w r9 r9b r9d r9w rax rbp rbx rcx rdi rdx rip rsi rsp si sp ss st st0 st1 st2 st3 st4 st5 st6 "
    "st7 tr3 tr4 tr5 tr6 tr7 xmm0 xmm1 xmm10 xmm11 xmm12 xmm13 xmm14 xmm15 xmm2 xmm3 xmm4 xmm5 xmm6 "
    "xmm7 xmm8 xmm9 ymm0 ymm1 ymm10 ymm11 ymm12 ymm13 ymm14 ymm15 ymm2 ymm3 ymm4 ymm5 ymm6 ymm7 ymm8 "
    "ymm9",
    "%arg %assign %define %elif %elifctk %elifdef %elifid %elifidn %elifidni %elifmacro %elifnctk "
    "%elifndef %elifnid %elifnidn %elifnidni %elifnmacro %elifnnum %elifnstr %elifnum %elifstr %else "
    "%endif %endmacro %endrep %error %exitrep %iassign %idefine %if %ifctk %ifdef %ifid %ifidn "
    "%ifidni %ifmacro %ifnctk %ifndef %ifnid %ifnidn %ifnidni %ifnmacro %ifnnum %ifnstr %ifnum %ifstr "
    "%imacro %include %line %local %macro %out %pop %push %rep %repl %rotate %stacksize %strlen "
    "%substr %undef %xdefine %xidefine .186 .286 .286c .286p .287 .386 .386c .386p .387 .486 .486p "
    ".8086 .8087 .alpha .break .code .const .continue .cref .data .data? .dosseg .else .elseif .endif "
    ".endw .err .err1 .err2 .errb .errdef .errdif .errdifi .erre .erridn .erridni .errnb .errndef "
    ".errnz .exit .fardata .fardata? .if .lall .lfcond .list .listall .listif .listmacro "
    ".listmacroall .model .msfloat .no87 .nocref .nolist .nolistif .nolistmacro .radix .repeat .sall "
    ".seq .sfcond .stack .startup .tfcond .type .until .untilcxz .while .xall .xcref .xlist absolute "
    "alias align alignb assume at bits catstr comm comment common cpu db dd df dosseg dq dt dup dw "
    "echo else elseif elseif1 elseif2 elseifb elseifdef elseifdif elseifdifi elseife elseifidn "
    "elseifidni elseifnb elseifndef end endif endm endp ends endstruc eq equ even exitm export extern "
    "externdef extrn for forc ge global goto group gt high highword iend if if1 if2 ifb ifdef ifdif "
    "ifdifi ife ifidn ifidni ifnb ifndef import incbin include includelib instr invoke irp irpc "
    "istruc label le length lengthof local low lowword lroffset lt macro mask mod name ne offset "
    "opattr option org page popcontext proc proto ptr public purge pushcontext record repeat rept "
    "resb resd resq rest resw section seg segment short size sizeof sizestr struc struct substr "
    "subtitle subttl textequ this times title type typedef union use16 use32 while width",
    "$ $$ %0 %1 %2 %3 %4 %5 %6 %7 %8 %9 .bss .data .text ? @b @f a16 a32 abs addr all assumes at "
    "basic byte c carry? casemap common compact cpu dotname dword emulator epilogue error export "
    "expr16 expr32 far far16 far32 farstack flat forceframe fortran fword huge language large listing "
    "ljmp loadds m510 medium memory near near16 near32 nearstack nodotname noemulator nokeyword "
    "noljmp nom510 none nonunique nooldmacros nooldstructs noreadonly noscoped nosignextend nosplit "
    "nothing notpublic o16 o32 oldmacros oldstructs os_dos overflow? para parity? pascal private "
    "prologue qword radix readonly real10 real4 real8 req sbyte scoped sdword seq setif2 sign? small "
    "smallstack stdcall sword syscall tbyte tiny use16 use32 uses vararg word wrt zero?",
    "addpd addps addsd addss andnpd andnps andpd andps blendpd blendps blendvpd blendvps cmpeqpd "
    "cmpeqps cmpeqsd cmpeqss cmplepd cmpleps cmplesd cmpless cmpltpd cmpltps cmpltsd cmpltss cmpnepd "
    "cmpneps cmpnesd cmpness cmpnlepd cmpnleps cmpnlesd cmpnless cmpnltpd cmpnltps cmpnltsd cmpnltss "
    "cmpordpd cmpordps cmpordsd cmpordss cmpunordpd cmpunordps cmpunordsd cmpunordss comisd comiss "
    "crc32 cvtdq2pd cvtdq2ps cvtpd2dq cvtpd2pi cvtpd2ps cvtpi2pd cvtpi2ps cvtps2dq cvtps2pd cvtps2pi "
    "cvtsd2si cvtsd2ss cvtsi2sd cvtsi2ss cvtss2sd cvtss2si cvttpd2dq cvttpd2pi cvttps2dq cvttps2pi "
    "cvttsd2si cvttss2si divpd divps divsd divss dppd dpps extractps fxrstor fxsave insertps ldmxscr "
    "lfence maskmovdq maskmovdqu maxpd maxps maxss mfence minpd minps minsd minss movapd movaps movd "
    "movdq2q movdqa movdqu movhlps movhpd movhps movlhps movlpd movlps movmskpd movmskps movntdq "
    "movntdqa movnti movntpd movntps movntq movq movq2dq movsd movss movupd movups mpsadbw mulpd "
    "mulps mulsd mulss orpd orps packssdw packsswb packusdw packuswb paddb paddd paddq paddsb paddsiw "
    "paddsw paddusb paddusw paddw pand pandn pause paveb pavgb pavgusb pavgw paxsd pblendvb pblendw "
    "pcmpeqb pcmpeqd pcmpeqq pcmpeqw pcmpestri pcmpestrm pcmpgtb pcmpgtd pcmpgtq pcmpgtw pcmpistri "
    "pcmpistrm pdistib pextrb pextrd pextrq pextrw pf2id pf2iw pfacc pfadd pfcmpeq pfcmpge pfcmpgt "
    "pfmax pfmin pfmul pfnacc pfpnacc pfrcp pfrcpit1 pfrcpit2 pfrsqit1 pfrsqrt pfsub pfsubr "
    "phminposuw pi2fd pinsrb pinsrd pinsrq pinsrw pmachriw pmaddwd pmagw pmaxsb pmaxsd pmaxsw pmaxub "
    "pmaxud pmaxuw pminsb pminsd pminsw pminub pminud pminuw pmovmskb pmovsxbd pmovsxbq pmovsxbw "
    "pmovsxdq pmovsxwd pmovsxwq pmovzxbd pmovzxbq pmovzxbw pmovzxdq pmovzxwd pmovzxwq pmuldq pmulhriw "
    "pmulhrwa pmulhrwc pmulhuw pmulhw pmulld pmullw pmuludq pmvgezb pmvlzb pmvnzb pmvzb popcnt por "
    "prefetch prefetchnta prefetcht0 prefetcht1 prefetcht2 prefetchw psadbw pshufd pshufhw pshuflw "
    "pshufw pslld pslldq psllq psllw psrad psraw psrld psrldq psrlq psrlw psubb psubd psubq psubsb "
    "psubsiw psubsw psubusb psubusw psubw pswapd ptest punpckhbw punpckhdq punpckhqdq punpckhwd "
    "punpcklbw punpckldq punpcklqdq punpcklwd pxor rcpps rcpss roundpd roundps roundsd roundss "
    "rsqrtps rsqrtss sfence shufpd shufps sqrtpd sqrtps sqrtsd sqrtss stmxcsr subpd subps subsd subss "
    "ucomisd ucomiss unpckhpd unpckhps unpcklpd unpcklps xorpd xorps",
    "", "", ""
};


EDITLEXER lexASM = { SCLEX_ASM, 63013, L"Assembly Script", L"asm", L"", &KeyWords_ASM, COMMENT_INFO(ASM_COMMENT, SCE_ASM_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_ASM_COMMENT, SCE_ASM_COMMENTBLOCK, 0, 0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_ASM_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_ASM_STRING, SCE_ASM_CHARACTER, SCE_ASM_STRINGEOL, 0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_ASM_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_ASM_OPERATOR, 63132, L"Operator", L"fore:#0A246A", L"" },
        { SCE_ASM_CPUINSTRUCTION, 63206, L"CPU Instruction", L"fore:#0A246A", L"" },
        { SCE_ASM_MATHINSTRUCTION, 63207, L"FPU Instruction", L"fore:#0A246A", L"" },
        { SCE_ASM_EXTINSTRUCTION, 63210, L"Extended Instruction", L"fore:#0A246A", L"" },
        { SCE_ASM_DIRECTIVE, 63203, L"Directive", L"fore:#0A246A", L"" },
        { SCE_ASM_DIRECTIVEOPERAND, 63209, L"Directive Operand", L"fore:#0A246A", L"" },
        { SCE_ASM_REGISTER, 63208, L"Register", L"fore:#FF8000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_PL = {
    "__DATA__ __END__ __FILE__ __LINE__ __PACKAGE__ abs accept alarm and atan2 AUTOLOAD BEGIN "
    "bind binmode bless caller chdir CHECK chmod chomp chop chown chr chroot close closedir cmp "
    "connect continue CORE cos crypt dbmclose dbmopen default defined delete DESTROY die do "
    "dump each else elsif END endgrent endhostent endnetent endprotoent endpwent endservent eof "
    "eq EQ eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge GE "
    "getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin "
    "getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname "
    "getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport "
    "getservent getsockname getsockopt given glob gmtime goto grep gt GT hex if index INIT int "
    "ioctl join keys kill last lc lcfirst le LE length link listen local localtime lock log "
    "lstat lt LT map mkdir msgctl msgget msgrcv msgsnd my ne NE next no not NULL oct open "
    "opendir or ord our pack package pipe pop pos print printf prototype push qu quotemeta rand "
    "read readdir readline readlink readpipe recv redo ref rename require reset return reverse "
    "rewinddir rindex rmdir say scalar seek seekdir select semctl semget semop send setgrent "
    "sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift "
    "shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split "
    "sprintf sqrt srand stat state study sub substr symlink syscall sysopen sysread sysseek "
    "system syswrite tell telldir tie tied time times truncate uc ucfirst umask undef UNITCHECK "
    "unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn "
    "when while write xor",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexPL = { SCLEX_PERL, 63014, L"Perl", L"pl; pm; cgi; pod", L"", &KeyWords_PL, COMMENT_INFO(BASH_COMMENT, SCE_PL_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_PL_COMMENTLINE, 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_PL_WORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
        { SCE_PL_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_PL_STRING, 63211, L"String double quoted", L"fore:#008000", L"" },
        { SCE_PL_CHARACTER, 63212, L"String single quoted", L"fore:#008000", L"" },
        { SCE_PL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_PL_OPERATOR, 63132, L"Operator", L"bold", L"" },
        { SCE_PL_SCALAR, 63215, L"Scalar $var", L"fore:#0A246A", L"" },
        { SCE_PL_ARRAY, 63216, L"Array @var", L"fore:#003CE6", L"" },
        { SCE_PL_HASH, 63217, L"Hash %var", L"fore:#B000B0", L"" },
        { SCE_PL_SYMBOLTABLE, 63218, L"Symbol table *var", L"fore:#3A6EA5", L"" },
        { SCE_PL_REGEX, 63219, L"Regex /re/ or m{re}", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_PL_REGSUBST, 63220, L"Substitution s/re/ore/", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_PL_BACKTICKS, 63221, L"Back ticks", L"fore:#E24000; back:#FFF1A8", L"" },
        { SCE_PL_HERE_DELIM, 63223, L"Here-doc (delimiter)", L"fore:#648000", L"" },
        { SCE_PL_HERE_Q, 63224, L"Here-doc (single quoted, q)", L"fore:#648000", L"" },
        { SCE_PL_HERE_QQ, 63225, L"Here-doc (double quoted, qq)", L"fore:#648000", L"" },
        { SCE_PL_HERE_QX, 63226, L"Here-doc (back ticks, qx)", L"fore:#E24000; back:#FFF1A8", L"" },
        { SCE_PL_STRING_Q, 63227, L"Single quoted string (generic, q)", L"fore:#008000", L"" },
        { SCE_PL_STRING_QQ, 63228, L"Double quoted string (qq)", L"fore:#008000", L"" },
        { SCE_PL_STRING_QX, 63229, L"Back ticks (qx)", L"fore:#E24000; back:#FFF1A8", L"" },
        { SCE_PL_STRING_QR, 63230, L"Regex (qr)", L"fore:#006633; back:#FFF1A8", L"" },
        { SCE_PL_STRING_QW, 63231, L"Array (qw)", L"fore:#003CE6", L"" },
        { SCE_PL_SUB_PROTOTYPE, 63253, L"Prototype", L"fore:#800080; back:#FFE2FF", L"" },
        { SCE_PL_FORMAT_IDENT, 63254, L"Format identifier", L"bold; fore:#648000; back:#FFF1A8", L"" },
        { SCE_PL_FORMAT, 63255, L"Format body", L"fore:#648000; back:#FFF1A8", L"" },
        { SCE_PL_POD, 63213, L"POD (common)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { SCE_PL_POD_VERB, 63214, L"POD (verbatim)", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { SCE_PL_DATASECTION, 63222, L"Data section", L"fore:#A46000; back:#FFFFC0; eolfilled", L"" },
        { SCE_PL_ERROR, 63252, L"Parsing error", L"fore:#C80000; back:#FFFF80", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_INI = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexINI = { SCLEX_PROPERTIES, 63015, L"Configuration Files (INI)", L"ini; inf; reg; cfg; properties; oem; sif; url; sed; theme", L"", &KeyWords_INI, COMMENT_INFO(ASM_COMMENT, SCE_PROPS_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_PROPS_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_PROPS_SECTION, 63232, L"Section", L"bold; fore:#000000; back:#FFD24D; eolfilled", L"" },
        { SCE_PROPS_ASSIGNMENT, 63233, L"Assignment", L"fore:#FF0000", L"" },
        { SCE_PROPS_DEFVAL, 63234, L"Default Value", L"fore:#FF0000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_BAT = {
    "break call cd chcp chdir choice cls color com con copy country date defined del dir "
    "disabledelayedexpansion disableextensions do doskey echo else enabledelayedexpansion "
    "enableextensions endlocal equ erase errorlevel exist exit for geq goto gtr if in leq "
    "loadfix loadhigh lpt lss md mkdir more move neq not nul off on path pause popd print "
    "prompt pushd rd rem ren rename rmdir set setlocal shift time title tree type ver verify",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexBAT = { SCLEX_BATCH, 63016, L"Batch Script", L"bat; cmd", L"", &KeyWords_BAT, COMMENT_INFO(BATCH_COMMENT, SCE_BAT_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_BAT_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_BAT_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
        { SCE_BAT_IDENTIFIER, 63129, L"Identifier", L"fore:#003CE6; back:#FFF1A8", L"" },
        { SCE_BAT_OPERATOR, 63132, L"Operator", L"", L"" },
        { MULTI_STYLE(SCE_BAT_COMMAND, SCE_BAT_HIDE, 0, 0), 63236, L"Command", L"bold", L"" },
        { SCE_BAT_LABEL, 63235, L"Label", L"fore:#C80000; back:#F4F4F4; eolfilled", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_DIFF = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexDIFF = { SCLEX_DIFF, 63017, L"Diff Files", L"diff; patch", L"", &KeyWords_DIFF, COMMENT_INFO(DIFF_COMMENT, SCE_DIFF_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_DIFF_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_DIFF_COMMAND, 63236, L"Command", L"bold; fore:#0A246A", L"" },
        { SCE_DIFF_HEADER, 63238, L"Source and Destination", L"fore:#C80000; back:#FFF1A8; eolfilled", L"" },
        { SCE_DIFF_POSITION, 63239, L"Position Setting", L"fore:#0000FF", L"" },
        { SCE_DIFF_ADDED, 63240, L"Line Addition", L"fore:#000000; back:#C0FF60; eolfilled", L"" },
        { SCE_DIFF_DELETED, 63241, L"Line Removal", L"fore:#000000; back:#FF8060; eolfilled", L"" },
        { SCE_DIFF_CHANGED, 63242, L"Line Change", L"fore:#000000; back:#99D7FF; eolfilled", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_SQL = {
    "abort accessible action add after all alter analyze and as asc asensitive attach autoincrement "
    "before begin between bigint binary bit blob both by call cascade case cast change char character "
    "check collate column commit condition conflict constraint continue convert create cross current_date "
    "current_time current_timestamp current_user cursor database databases date day_hour day_microsecond "
    "day_minute day_second dec decimal declare default deferrable deferred delayed delete desc describe "
    "detach deterministic distinct distinctrow div double drop dual each else elseif enclosed end enum "
    "escape escaped except exclusive exists exit explain fail false fetch float float4 float8 for force "
    "foreign from full fulltext glob grant group having high_priority hour_microsecond hour_minute "
    "hour_second if ignore immediate in index infile initially inner inout insensitive insert instead int "
    "int1 int2 int3 int4 int8 integer intersect interval into is isnull iterate join key keys kill "
    "leading leave left like limit linear lines load localtime localtimestamp lock long longblob longtext "
    "loop low_priority master_ssl_verify_server_cert match mediumblob mediumint mediumtext middleint "
    "minute_microsecond minute_second mod modifies natural no no_write_to_binlog not notnull null numeric "
    "of offset on optimize option optionally or order out outer outfile plan pragma precision primary "
    "procedure purge query raise range read read_only read_write reads real references regexp reindex "
    "release rename repeat replace require restrict return revoke right rlike rollback row rowid schema "
    "schemas second_microsecond select sensitive separator set show smallint spatial specific sql "
    "sql_big_result sql_calc_found_rows sql_small_result sqlexception sqlstate sqlwarning ssl starting "
    "straight_join table temp temporary terminated text then time timestamp tinyblob tinyint tinytext to "
    "trailing transaction trigger true undo union unique unlock unsigned update usage use using utc_date "
    "utc_time utc_timestamp vacuum values varbinary varchar varcharacter varying view virtual when where "
    "while with write xor year_month zerofill",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexSQL = { SCLEX_SQL, 63018, L"SQL Query", L"sql", L"", &KeyWords_SQL, COMMENT_INFO(SQL_COMMENT, -1), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE2(SCE_SQL_COMMENT, SCE_SQL_COMMENTLINE, SCE_SQL_COMMENTDOC, SCE_SQL_COMMENTLINEDOC, SCE_SQL_COMMENTDOCKEYWORD, SCE_SQL_COMMENTDOCKEYWORDERROR, 0, 0), 63127, L"Comment", L"fore:#505050", L"" },
        { SCE_SQL_WORD, 63128, L"Keyword", L"bold; fore:#800080", L"" },
        { MULTI_STYLE(SCE_SQL_STRING, SCE_SQL_CHARACTER, 0, 0), 63131, L"String", L"fore:#008000; back:#FFF1A8", L"" },
        { SCE_SQL_IDENTIFIER, 63129, L"Identifier", L"fore:#800080", L"" },
        { SCE_SQL_QUOTEDIDENTIFIER, 63243, L"Quoted Identifier", L"fore:#800080; back:#FFCCFF", L"" },
        { SCE_SQL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_SQL_OPERATOR, 63132, L"Operator", L"bold; fore:#800080", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_PY = {
    "and as assert break class continue def del elif else except "
    "exec False finally for from global if import in is lambda None "
    "not or pass print raise return True try with while yield",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexPY = { SCLEX_PYTHON, 63019, L"Python", L"py; pyw", L"", &KeyWords_PY, COMMENT_INFO(BASH_COMMENT, SCE_P_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_P_COMMENTLINE, SCE_P_COMMENTBLOCK, 0, 0), 63127, L"Comment", L"fore:#880000", L"" },
        { SCE_P_WORD, 63128, L"Keyword", L"fore:#000088", L"" },
        { SCE_P_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_P_STRING, SCE_P_STRINGEOL, 0, 0), 63211, L"String double quoted", L"fore:#008800", L"" },
        { SCE_P_CHARACTER, 63212, L"String single quoted", L"fore:#008800", L"" },
        { SCE_P_TRIPLEDOUBLE, 63244, L"String triple double quotes", L"fore:#008800", L"" },
        { SCE_P_TRIPLE, 63245, L"String triple single quotes", L"fore:#008800", L"" },
        { SCE_P_NUMBER, 63130, L"Number", L"fore:#FF4000", L"" },
        { SCE_P_OPERATOR, 63132, L"Operator", L"bold; fore:#666600", L"" },
        { SCE_P_DEFNAME, 63247, L"Function name", L"fore:#660066", L"" },
        { SCE_P_CLASSNAME, 63246, L"Class name", L"fore:#660066", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_CONF = {
    "acceptmutex acceptpathinfo accessconfig accessfilename action addalt addaltbyencoding addaltbytype addcharset adddefaultcharset "
    "adddescription addencoding addhandler addicon addiconbyencoding addiconbytype addinputfilter addlanguage addmodule addmoduleinfo "
    "addoutputfilter addoutputfilterbytype addtype agentlog alias aliasmatch all allow allowconnect allowencodedslashes allowoverride "
    "anonymous anonymous_authoritative anonymous_logemail anonymous_mustgiveemail anonymous_nouserid anonymous_verifyemail "
    "assignuserid authauthoritative authdbauthoritative authdbgroupfile authdbmauthoritative authdbmgroupfile authdbmtype "
    "authdbmuserfile authdbuserfile authdigestalgorithm authdigestdomain authdigestfile authdigestgroupfile authdigestnccheck "
    "authdigestnonceformat authdigestnoncelifetime authdigestqop authdigestshmemsize authgroupfile authldapauthoritative "
    "authldapbinddn authldapbindpassword authldapcharsetconfig authldapcomparednonserver authldapdereferencealiases authldapenabled "
    "authldapfrontpagehack authldapgroupattribute authldapgroupattributeisdn authldapremoteuserisdn authldapurl authname authtype "
    "authuserfile bindaddress browsermatch browsermatchnocase bs2000account bufferedlogs cachedefaultexpire cachedirlength "
    "cachedirlevels cachedisable cacheenable cacheexpirycheck cachefile cacheforcecompletion cachegcclean cachegcdaily "
    "cachegcinterval cachegcmemusage cachegcunused cacheignorecachecontrol cacheignoreheaders cacheignorenolastmod "
    "cachelastmodifiedfactor cachemaxexpire cachemaxfilesize cacheminfilesize cachenegotiateddocs cacheroot cachesize cachetimemargin "
    "cgimapextension charsetdefault charsetoptions charsetsourceenc checkspelling childperuserid clearmodulelist contentdigest "
    "cookiedomain cookieexpires cookielog cookiename cookiestyle cookietracking coredumpdirectory customlog dav davdepthinfinity "
    "davlockdb davmintimeout defaulticon defaultlanguage defaulttype define deflatebuffersize deflatecompressionlevel "
    "deflatefilternote deflatememlevel deflatewindowsize deny directory directoryindex directorymatch directoryslash documentroot "
    "dumpioinput dumpiooutput enableexceptionhook enablemmap enablesendfile errordocument errorlog example expiresactive "
    "expiresbytype expiresdefault extendedstatus extfilterdefine extfilteroptions fancyindexing fileetag files filesmatch "
    "forcelanguagepriority forcetype forensiclog from group header headername hostnamelookups identitycheck ifdefine ifmodule "
    "imapbase imapdefault imapmenu include indexignore indexoptions indexorderdefault isapiappendlogtoerrors isapiappendlogtoquery "
    "isapicachefile isapifakeasync isapilognotsupported isapireadaheadbuffer keepalive keepalivetimeout languagepriority "
    "ldapcacheentries ldapcachettl ldapconnectiontimeout ldapopcacheentries ldapopcachettl ldapsharedcachefile ldapsharedcachesize "
    "ldaptrustedca ldaptrustedcatype limit limitexcept limitinternalrecursion limitrequestbody limitrequestfields "
    "limitrequestfieldsize limitrequestline limitxmlrequestbody listen listenbacklog loadfile loadmodule location locationmatch "
    "lockfile logformat loglevel maxclients maxkeepaliverequests maxmemfree maxrequestsperchild maxrequestsperthread maxspareservers "
    "maxsparethreads maxthreads maxthreadsperchild mcachemaxobjectcount mcachemaxobjectsize mcachemaxstreamingbuffer "
    "mcacheminobjectsize mcacheremovalalgorithm mcachesize metadir metafiles metasuffix mimemagicfile minspareservers minsparethreads "
    "mmapfile modmimeusepathinfo multiviewsmatch namevirtualhost nocache noproxy numservers nwssltrustedcerts nwsslupgradeable "
    "options order passenv pidfile port protocolecho proxy proxybadheader proxyblock proxydomain proxyerroroverride proxyiobuffersize "
    "proxymatch proxymaxforwards proxypass proxypassreverse proxypreservehost proxyreceivebuffersize proxyremote proxyremotematch "
    "proxyrequests proxytimeout proxyvia qsc readmename redirect redirectmatch redirectpermanent redirecttemp refererignore "
    "refererlog removecharset removeencoding removehandler removeinputfilter removelanguage removeoutputfilter removetype "
    "requestheader require resourceconfig rewritebase rewritecond rewriteengine rewritelock rewritelog rewriteloglevel rewritemap "
    "rewriteoptions rewriterule rlimitcpu rlimitmem rlimitnproc satisfy scoreboardfile script scriptalias scriptaliasmatch "
    "scriptinterpretersource scriptlog scriptlogbuffer scriptloglength scriptsock securelisten sendbuffersize serveradmin serveralias "
    "serverlimit servername serverpath serverroot serversignature servertokens servertype setenv setenvif setenvifnocase sethandler "
    "setinputfilter setoutputfilter singlelisten ssiendtag ssierrormsg ssistarttag ssitimeformat ssiundefinedecho "
    "sslcacertificatefile sslcacertificatepath sslcarevocationfile sslcarevocationpath sslcertificatechainfile sslcertificatefile "
    "sslcertificatekeyfile sslciphersuite sslengine sslmutex ssloptions sslpassphrasedialog sslprotocol sslproxycacertificatefile "
    "sslproxycacertificatepath sslproxycarevocationfile sslproxycarevocationpath sslproxyciphersuite sslproxyengine "
    "sslproxymachinecertificatefile sslproxymachinecertificatepath sslproxyprotocol sslproxyverify sslproxyverifydepth sslrandomseed "
    "sslrequire sslrequiressl sslsessioncache sslsessioncachetimeout sslusername sslverifyclient sslverifydepth startservers "
    "startthreads suexecusergroup threadlimit threadsperchild threadstacksize timeout transferlog typesconfig unsetenv "
    "usecanonicalname user userdir virtualdocumentroot virtualdocumentrootip virtualhost virtualscriptalias virtualscriptaliasip "
    "win32disableacceptex xbithack",
    "",
    "", "", "", "", "", "", ""
};


EDITLEXER lexCONF = { SCLEX_CONF, 63020, L"Apache Config File", L"conf; htaccess", L"", &KeyWords_CONF, COMMENT_INFO(BASH_COMMENT, SCE_CONF_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_CONF_COMMENT, 63127, L"Comment", L"fore:#648000", L"" },
        { SCE_CONF_STRING, 63131, L"String", L"fore:#B000B0", L"" },
        { SCE_CONF_NUMBER, 63130, L"Number", L"fore:#FF4000", L"" },
        { SCE_CONF_DIRECTIVE, 63203, L"Directive", L"fore:#003CE6", L"" },
        { SCE_CONF_IP, 63248, L"IP Address", L"bold; fore:#FF4000", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_PS = {
    "begin break catch continue data do dynamicparam else elseif end exit filter finally for foreach "
    "from function if in local param private process return switch throw trap try until where while",
    "add-computer add-content add-history add-member add-pssnapin add-type checkpoint-computer "
    "clear-content clear-eventlog clear-history clear-host clear-item clear-itemproperty "
    "clear-variable compare-object complete-transaction connect-wsman convertfrom-csv "
    "convertfrom-securestring convertfrom-stringdata convert-path convertto-csv convertto-html "
    "convertto-securestring convertto-xml copy-item copy-itemproperty debug-process "
    "disable-computerrestore disable-psbreakpoint disable-psremoting disable-pssessionconfiguration "
    "disable-wsmancredssp disconnect-wsman enable-computerrestore enable-psbreakpoint "
    "enable-psremoting enable-pssessionconfiguration enable-wsmancredssp enter-pssession "
    "exit-pssession export-alias export-clixml export-console export-counter export-csv "
    "export-formatdata export-modulemember export-pssession foreach-object format-custom format-list "
    "format-table format-wide get-acl get-alias get-authenticodesignature get-childitem get-command "
    "get-computerrestorepoint get-content get-counter get-credential get-culture get-date get-event "
    "get-eventlog get-eventsubscriber get-executionpolicy get-formatdata get-help get-history "
    "get-host get-hotfix get-item get-itemproperty get-job get-location get-member get-module "
    "get-pfxcertificate get-process get-psbreakpoint get-pscallstack get-psdrive get-psprovider "
    "get-pssession get-pssessionconfiguration get-pssnapin get-random get-service get-tracesource "
    "get-transaction get-uiculture get-unique get-variable get-verb get-winevent get-wmiobject "
    "get-wsmancredssp get-wsmaninstance group-object import-alias import-clixml import-counter "
    "import-csv import-localizeddata import-module import-pssession invoke-command invoke-expression "
    "invoke-history invoke-item invoke-wmimethod invoke-wsmanaction join-path limit-eventlog "
    "measure-command measure-object move-item move-itemproperty new-alias new-event new-eventlog "
    "new-item new-itemproperty new-module new-modulemanifest new-object new-psdrive new-pssession "
    "new-pssessionoption new-service new-timespan new-variable new-webserviceproxy new-wsmaninstance "
    "new-wsmansessionoption out-default out-file out-gridview out-host out-null out-printer "
    "out-string pop-location push-location read-host receive-job register-engineevent "
    "register-objectevent register-pssessionconfiguration register-wmievent remove-computer "
    "remove-event remove-eventlog remove-item remove-itemproperty remove-job remove-module "
    "remove-psbreakpoint remove-psdrive remove-pssession remove-pssnapin remove-variable "
    "remove-wmiobject remove-wsmaninstance rename-item rename-itemproperty "
    "reset-computermachinepassword resolve-path restart-computer restart-service restore-computer "
    "resume-service select-object select-string select-xml send-mailmessage set-acl set-alias "
    "set-authenticodesignature set-content set-date set-executionpolicy set-item set-itemproperty "
    "set-location set-psbreakpoint set-psdebug set-pssessionconfiguration set-service set-strictmode "
    "set-tracesource set-variable set-wmiinstance set-wsmaninstance set-wsmanquickconfig "
    "show-eventlog sort-object split-path start-job start-process start-service start-sleep "
    "start-transaction start-transcript stop-computer stop-job stop-process stop-service "
    "stop-transcript suspend-service tee-object test-computersecurechannel test-connection "
    "test-modulemanifest test-path test-wsman trace-command undo-transaction unregister-event "
    "unregister-pssessionconfiguration update-formatdata update-list update-typedata use-transaction "
    "wait-event wait-job wait-process where-object write-debug write-error write-eventlog write-host "
    "write-output write-progress write-verbose write-warning",
    "ac asnp cat cd chdir clc clear clhy cli clp cls clv compare copy cp cpi cpp cvpa dbp del diff "
    "dir ebp echo epal epcsv epsn erase etsn exsn fc fl foreach ft fw gal gbp gc gci gcm gcs gdr ghy "
    "gi gjb gl gm gmo gp gps group gsn gsnp gsv gu gv gwmi h help history icm iex ihy ii ipal ipcsv "
    "ipmo ipsn ise iwmi kill lp ls man md measure mi mkdir more mount move mp mv nal ndr ni nmo nsn "
    "nv ogv oh popd ps pushd pwd r rbp rcjb rd rdr ren ri rjb rm rmdir rmo rni rnp rp rsn rsnp rv "
    "rvpa rwmi sajb sal saps sasv sbp sc select set si sl sleep sort sp spjb spps spsv start sv swmi "
    "tee type where wjb write",
    "importsystemmodules prompt psedit tabexpansion",
    "", "", "", "", ""
};


EDITLEXER lexPS = { SCLEX_POWERSHELL, 63021, L"PowerShell Script", L"ps1; psd1; psm1", L"", &KeyWords_PS, COMMENT_INFO(BASH_COMMENT, SCE_POWERSHELL_COMMENT), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_POWERSHELL_COMMENT, SCE_POWERSHELL_COMMENTSTREAM, SCE_POWERSHELL_COMMENTDOCKEYWORD, 0), 63127, L"Comment", L"fore:#646464", L"" },
        { SCE_POWERSHELL_KEYWORD, 63128, L"Keyword", L"bold; fore:#804000", L"" },
        { SCE_POWERSHELL_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_POWERSHELL_STRING, SCE_POWERSHELL_CHARACTER, 0, 0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_POWERSHELL_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
        { SCE_POWERSHELL_OPERATOR, 63132, L"Operator", L"bold", L"" },
        { SCE_POWERSHELL_VARIABLE, 63249, L"Variable", L"fore:#0A246A", L"" },
        { MULTI_STYLE(SCE_POWERSHELL_CMDLET, SCE_POWERSHELL_FUNCTION, 0, 0), 63250, L"Cmdlet", L"fore:#804000; back:#FFF1A8", L"" },
        { SCE_POWERSHELL_ALIAS, 63251, L"Alias", L"bold; fore:#0A246A", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_RUBY = {
    "__FILE__ __LINE__ alias and begin break case class def defined? do else elsif end ensure "
    "false for in if module next nil not or redo rescue retry return self super then true "
    "undef unless until when while yield",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexRUBY = { SCLEX_RUBY, 63022, L"Ruby", L"rb; ruby; rbw; rake; rjs; Rakefile", L"", &KeyWords_RUBY, COMMENT_INFO(BASH_COMMENT, SCE_RB_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_RB_COMMENTLINE, SCE_P_COMMENTBLOCK, 0, 0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_RB_WORD, 63128, L"Keyword", L"bold; fore:#00007F", L"" },
        { SCE_RB_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_RB_NUMBER, 63130, L"Number", L"fore:#008080", L"" },
        { SCE_RB_OPERATOR, 63132, L"Operator", L"", L"" },
        { MULTI_STYLE(SCE_RB_STRING, SCE_RB_CHARACTER, SCE_P_STRINGEOL, 0), 63131, L"String", L"fore:#FF8000", L"" },
        { SCE_RB_CLASSNAME, 63246, L"Class name", L"fore:#0000FF", L"" },
        { SCE_RB_DEFNAME, 63247, L"Function name", L"fore:#007F7F", L"" },
        { SCE_RB_POD, 63314, L"POD", L"fore:#004000; back:#C0FFC0; eolfilled", L"" },
        { SCE_RB_REGEX, 63135, L"Regex", L"fore:#000000; back:#A0FFA0", L"" },
        { SCE_RB_SYMBOL, 63316, L"Symbol", L"fore:#C0A030", L"" },
        { SCE_RB_MODULE_NAME, 63317, L"Module name", L"fore:#A000A0", L"" },
        { SCE_RB_INSTANCE_VAR, 63318, L"Instance Var", L"fore:#B00080", L"" },
        { SCE_RB_CLASS_VAR, 63319, L"Class Var", L"fore:#8000B0", L"" },
        { SCE_RB_DATASECTION, 63320, L"Data Section", L"fore:#600000; back:#FFF0D8; eolfilled", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_BASH = {
    "alias "
    "ar asa awk banner basename bash bc bdiff break "
    "bunzip2 bzip2 cal calendar case cat cc cd chmod cksum "
    "clear cmp col comm compress continue cp cpio crypt "
    "csplit ctags cut date dc dd declare deroff dev df diff diff3 "
    "dircmp dirname do done du echo ed egrep elif else env "
    "esac eval ex exec exit expand export expr false fc "
    "fgrep fi file find fmt fold for function functions "
    "getconf getopt getopts grep gres hash head help "
    "history iconv id if in integer jobs join kill local lc "
    "let line ln logname look ls m4 mail mailx make "
    "man mkdir more mt mv newgrp nl nm nohup ntps od "
    "pack paste patch pathchk pax pcat perl pg pr print "
    "printf ps pwd read readonly red return rev rm rmdir "
    "sed select set sh shift size sleep sort spell "
    "split start stop strings strip stty sum suspend "
    "sync tail tar tee test then time times touch tr "
    "trap true tsort tty type typeset ulimit umask unalias "
    "uname uncompress unexpand uniq unpack unset until "
    "uudecode uuencode vi vim vpax wait wc whence which "
    "while who wpaste wstart xargs zcat "
    "chgrp chown chroot dir dircolors "
    "factor groups hostid install link md5sum mkfifo "
    "mknod nice pinky printenv ptx readlink seq "
    "sha1sum shred stat su tac unlink users vdir whoami yes",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexBASH = { SCLEX_BASH, 63023, L"Bash Script", L"sh; bash; configure; ksh", L"", &KeyWords_BASH, COMMENT_INFO(BASH_COMMENT, SCE_SH_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_SH_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_SH_ERROR, 63325, L"Error", L"fore:#FFFF00,back:#FF0000", L"" },
        { SCE_SH_COMMENTLINE, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_SH_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_SH_WORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_SH_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_SH_CHARACTER, 63265, L"Char", L"fore:#7F007F", L"" },
        { SCE_SH_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_SH_IDENTIFIER, 63129, L"Identifier", L"fore:#000000", L"" },
        { SCE_SH_SCALAR, 63215, L"Scalar $var", L"fore:#000000,back:#FFE0E0", L"" },
        { SCE_SH_PARAM, 63125, L"Parameter", L"fore:#000000,back:#FFFFE0", L"" },
        { SCE_SH_BACKTICKS, 63221, L"Back ticks", L"fore:#FFFF00,back:#A08080", L"" },
        { SCE_SH_HERE_DELIM, 63223, L"Here-doc (delimiter)", L"fore:#000000,back:#DDD0DD", L"" },
        { SCE_SH_HERE_Q, 63224, L"Here-doc (single quoted, q)", L"fore:#7F007F,back:#DDD0DD,eolfilled,notbold", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_ASN1 = {
    "ACCESS AGENT AUGMENTS "
    "BEGIN BITS "
    "CAPABILITIES CHOICE COMPLIANCE CONTACT CONVENTION "
    "DEFINITIONS DEFVAL DESCRIPTION DISPLAY "
    "END ENTERPRISE EXPORTS "
    "FALSE FROM "
    "GROUP GROUPS "
    "HINT "
    "IDENTITY IMPLIED IMPORTS INCLUDES INDEX INFO "
    "LAST "
    "MANDATORY MAX MIN MODULE "
    "NOTATION NOTIFICATION NULL "
    "OBJECTS OBJECT-TYPE OF ORGANIZATION "
    "PRODUCT "
    "RELEASE REFERENCE REQUIRES REVISION "
    "SEQUENCE SIZE STATUS SUPPORTS SYNTAX "
    "TEXTUAL TRAP TYPE TRAP-TYPE "
    "UPDATED "
    "VALUE VARIABLES VARIATION "
    "WRITE "
    "accessible "
    "create current "
    "deprecated "
    "for "
    "mandatory "
    "not notify not-accessible "
    "obsolete only optional "
    "read read-only read-write "
    "write "
    "ABSENT ANY APPLICATION "
    "BIT BOOLEAN BY "
    "COMPONENT COMPONENTS "
    "DEFAULT DEFINED "
    "ENUMERATED EXPLICIT EXTERNAL "
    "IMPLICIT INIFINITY "
    "MAX MIN MINUS "
    "OPTIONAL "
    "PRESENT PRIVATE "
    "REAL "
    "SET "
    "TAGS TRUE "
    "Counter Counter32 Counter64 "
    "DisplayString "
    "Gauge Gauge32 "
    "IDENTIFIER INTEGER Integer32 IpAddress "
    "NetworkAddress NsapAddress "
    "OBJECT OCTET Opaque "
    "PhysAddress "
    "STRING "
    "TimeTicks "
    "UInteger32 UNITS Unsigned32",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexASN1 = { SCLEX_ASN1, 63024, L"ASN1", L"mib", L"", &KeyWords_ASN1, COMMENT_INFO(SQL_COMMENT, SCE_ASN1_COMMENT), {
        { SCE_ASN1_DEFAULT, 63126, L"Default", L"fore:#000000", L"" },
        { SCE_ASN1_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_ASN1_IDENTIFIER, 63129, L"Identifier", L"fore:#000000,bold", L"" },
        { SCE_ASN1_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_ASN1_OID, 63129, L"Identifier", L"fore:#007F7F,bold", L"" },
        { SCE_ASN1_SCALAR, 63215, L"Scalar $var", L"fore:#7F0000", L"" },
        { SCE_ASN1_KEYWORD, 63128, L"Keyword", L"fore:#00007F", L"" },
        { SCE_ASN1_ATTRIBUTE, 63258, L"Attribute", L"fore:#F07800", L"" },
        { SCE_ASN1_DESCRIPTOR, 63259, L"Descriptor", L"fore:#00007F", L"" },
        { SCE_ASN1_TYPE, 63260, L"Type", L"fore:#00007F", L"" },
        { SCE_ASN1_OPERATOR, 63132, L"Operator", L"fore:#00007F", L"" },
        { -1, 00000, L"", L"", L"" }
    }
};


KEYWORDLIST KeyWords_CAML = {
    "and as assert asr begin class "
    "constraint do done downto else end "
    "exception external false for fun function "
    "functor if in include inherit initializer "
    "land lazy let lor lsl lsr "
    "lxor match method mod module mutable "
    "new object of open or private "
    "rec sig struct then to true "
    "try type val virtual when while "
    "with ",
    "option Some None ignore ref lnot succ pred parser ",
    "array bool char float int list string unit",
    "", "", "", "", "", ""
};


EDITLEXER lexCAML = { SCLEX_CAML, 63025, L"OCaml", L"ml; mli", L"", &KeyWords_CAML, NULL_COMMENT_INFO, {
        { SCE_CAML_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_CAML_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_CAML_TAGNAME, 63261, L"Tag Name", L"fore:#000000,back:#ffe0ff", L"" },
        { SCE_CAML_KEYWORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_CAML_KEYWORD2, 63262, L"Keyword 2", L"fore:#000000,back:#e0e0ff", L"" },
        { SCE_CAML_KEYWORD3, 63263, L"Keyword 3", L"fore:#a0000,bold", L"" },
        { SCE_CAML_LINENUM, 63264, L"Linenum", L"back:#C0C0C0", L"" },
        { SCE_CAML_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_CAML_NUMBER, 63130, L"Number", L"fore:#000000,back:#ffff00", L"" },
        { SCE_CAML_CHAR, 63265, L"Char", L"fore:#000000,back:#ffff00", L"" },
        { SCE_CAML_WHITE, 63266, L"Whitespace", L"fore:#000000,back:#e0e0e0", L"" },
        { SCE_CAML_STRING, 63131, L"String", L"fore:#000000,back:#ffff00", L"" },
        { SCE_CAML_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_CAML_COMMENT1, 63267, L"Comment 1", L"back:#E0EEFF", L"" },
        { SCE_CAML_COMMENT2, 63268, L"Comment 2", L"back:#E0EEFF", L"" },
        { SCE_CAML_COMMENT3, 63269, L"Comment 3", L"back:#E0EEFF", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_COFFEESCRIPT = {
      "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexCOFFEESCRIPT = { SCLEX_COFFEESCRIPT, 63026, L"CoffeeScript", L"coffee", L"", &KeyWords_COFFEESCRIPT, COMMENT_INFO(BATCH_COMMENT, SCE_COFFEESCRIPT_COMMENTLINE), {
        { SCE_COFFEESCRIPT_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_COFFEESCRIPT_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_COFFEESCRIPT_COMMENTLINE, 63270, L"Comment Line", L"fore:#007F00", L"" },
        { SCE_COFFEESCRIPT_COMMENTDOC, 63271, L"Comment Doc", L"fore:#3F703F", L"" },
        { SCE_COFFEESCRIPT_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_COFFEESCRIPT_WORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_COFFEESCRIPT_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_COFFEESCRIPT_CHARACTER, 63265, L"Char", L"fore:#7F007F", L"" },
        { SCE_COFFEESCRIPT_UUID, 63272, L"UUID", L"fore:#804080", L"" },
        { SCE_COFFEESCRIPT_PREPROCESSOR, 63273, L"Preprocessor", L"fore:#7F7F00", L"" },
        { SCE_COFFEESCRIPT_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_COFFEESCRIPT_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_COFFEESCRIPT_STRINGEOL, 63274, L"String EOL", L"fore:#000000,back:#E0C0E0,eolfilled", L"" },
        { SCE_COFFEESCRIPT_VERBATIM, 63275, L"Verbatim", L"fore:#007F00,back:#E0FFE0,eolfilled", L"" },
        { SCE_COFFEESCRIPT_REGEX, 63135, L"Regex", L"fore:#3F7F3F,back:#E0F0FF,eolfilled", L"" },
        { SCE_COFFEESCRIPT_COMMENTLINEDOC, 63276, L"Comment Line Doc", L"fore:#3F703F", L"" },
        { SCE_COFFEESCRIPT_WORD2, 63262, L"Keyword 2", L"fore:#B00040", L"" },
        { SCE_COFFEESCRIPT_COMMENTDOCKEYWORD, 63277, L"Comment Doc Keyword", L"fore:#3060A0", L"" },
        { SCE_COFFEESCRIPT_COMMENTDOCKEYWORDERROR, 63278, L"Comment Doc Keyword Error", L"fore:#804020", L"" },
        { SCE_COFFEESCRIPT_GLOBALCLASS, 63279, L"Global Class", L"fore:#DD9900", L"" },
        { SCE_COFFEESCRIPT_STRINGRAW, 63280, L"String Raw", L"fore:#7F007F,back:#FFF3FF,eolfilled", L"" },
        { SCE_COFFEESCRIPT_TRIPLEVERBATIM, 63281, L"Trip Lever Batim", L"fore:#007F00,back:#E0FFE0,eolfilled", L"" },
        { SCE_COFFEESCRIPT_COMMENTBLOCK, 63282, L"Comment Block", L"fore:#007F00,back:#E7FFD7,eolfilled", L"" },
        { SCE_COFFEESCRIPT_VERBOSE_REGEX, 63283, L"Verbose Regex", L"fore:#659900", L"" },
        { SCE_COFFEESCRIPT_VERBOSE_REGEX_COMMENT, 63284, L"Regex Comment", L"fore:#3F703F", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_D = {
    "abstract alias align asm assert auto "
    "body bool break byte "
    "case cast catch cdouble cent cfloat char class const continue creal "
    "dchar debug default delegate delete deprecated do double "
    "else enum export extern "
    "false final finally float for foreach foreach_reverse function "
    "goto "
    "idouble if ifloat import in inout int interface invariant ireal is "
    "lazy long "
    "mixin module "
    "new null "
    "out override "
    "package pragma private protected public "
    "real return "
    "scope short static struct super switch synchronized "
    "template this throw true try typedef typeid typeof "
    "ubyte ucent uint ulong union unittest ushort "
    "version void volatile "
    "wchar while with",
    "",
    "a addindex addtogroup anchor arg attention "
    "author b brief bug c class code date def defgroup deprecated dontinclude "
    "e em endcode endhtmlonly endif endlatexonly endlink endverbatim enum example exception "
    "f$ f[ f] file fn hideinitializer htmlinclude htmlonly "
    "if image include ingroup internal invariant interface latexonly li line link "
    "mainpage name namespace nosubgrouping note overload "
    "p page par param post pre ref relates remarks return retval "
    "sa section see showinitializer since skip skipline struct subsection "
    "test throw todo typedef union until "
    "var verbatim verbinclude version warning weakgroup $ @ \\ & < > # { }",
    "", "", "", "", "", ""
};


EDITLEXER lexD = { SCLEX_D, 63027, L"D", L"d", L"", &KeyWords_D, COMMENT_INFO(C_COMMENT, SCE_D_COMMENTLINE), {
        { SCE_D_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_D_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_D_COMMENTLINE, 63270, L"Comment Line", L"fore:#007F00", L"" },
        { SCE_D_COMMENTDOC, 63271, L"Comment Doc", L"fore:#3F703F", L"" },
        { SCE_D_COMMENTNESTED, 63286, L"Comment Nested", L"fore:#A0C0A0", L"" },
        { SCE_D_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_D_WORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_D_WORD2, 63262, L"Keyword 2", L"fore:#00007F,bold", L"" },
        { SCE_D_WORD3, 63263, L"Keyword 3", L"fore:#00007F,bold", L"" },
        { SCE_D_TYPEDEF, 63285, L"Typedef", L"fore:#00007F,bold", L"" },
        { SCE_D_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_D_STRINGEOL, 63274, L"String EOL", L"fore:#000000,back:#E0C0E0,eolfilled", L"" },
        { SCE_D_CHARACTER, 63265, L"Char", L"fore:#7F007F", L"" },
        { SCE_D_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_D_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_D_COMMENTLINEDOC, 63276, L"Comment Line Doc", L"fore:#3F703F", L"" },
        { SCE_D_COMMENTDOCKEYWORD, 63277, L"Comment Doc Keyword", L"fore:#3060A0", L"" },
        { SCE_D_COMMENTDOCKEYWORDERROR, 63278, L"Comment Doc Keyword Error", L"fore:#804020", L"" },
        { SCE_D_STRINGB, 63287, L"String B", L"fore:#DD9900", L"" },
        { SCE_D_STRINGR, 63288, L"String R", L"fore:#DD9900", L"" },
        { SCE_D_WORD5, 63290, L"Keyword 5", L"fore:#DD9900", L"" },
        { SCE_D_WORD6, 63291, L"Keyword 6", L"fore:#DD9900", L"" },
        { SCE_D_WORD7, 63292, L"Keyword 7", L"fore:#DD9900", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_Lisp = {
    "not defun + - * / = < > <= >= princ "
    "eval apply funcall quote identity function complement backquote lambda set setq setf "
    "defun defmacro gensym make symbol intern symbol name symbol value symbol plist get "
    "getf putprop remprop hash make array aref car cdr caar cadr cdar cddr caaar caadr cadar "
    "caddr cdaar cdadr cddar cdddr caaaar caaadr caadar caaddr cadaar cadadr caddar cadddr "
    "cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar cddddr cons list append reverse last nth "
    "nthcdr member assoc subst sublis nsubst  nsublis remove length list length "
    "mapc mapcar mapl maplist mapcan mapcon rplaca rplacd nconc delete atom symbolp numberp "
    "boundp null listp consp minusp zerop plusp evenp oddp eq eql equal cond case and or let l if prog "
    "prog1 prog2 progn go return do dolist dotimes catch throw error cerror break "
    "continue errset baktrace evalhook truncate float rem min max abs sin cos tan expt exp sqrt "
    "random logand logior logxor lognot bignums logeqv lognand lognor "
    "logorc2 logtest logbitp logcount integer length nil",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexLisp = { SCLEX_LISP, 63028, L"Lisp", L"lsp;lisp", L"", &KeyWords_Lisp, COMMENT_INFO(ASM_COMMENT, SCE_LISP_COMMENT), {
        { SCE_LISP_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_LISP_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_LISP_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_LISP_KEYWORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_LISP_KEYWORD_KW, 63262, L"Keyword 2", L"fore:#EE00AA", L"" },
        { SCE_LISP_SYMBOL, 63294, L"Symbol", L"fore:#DD9900", L"" },
        { SCE_LISP_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_LISP_STRINGEOL, 63274, L"String EOL", L"fore:#000000,back:#fefecc,eolfilled", L"" },
        { SCE_LISP_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_LISP_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_LISP_SPECIAL, 63295, L"Special", L"fore:#60AA00", L"" },
        { SCE_LISP_MULTI_COMMENT, 63296, L"Multi-line Comment", L"fore:#007F00", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_Lua = {
    "and break do else elseif "
    "end for function if in "
    "local nil not or repeat "
    "return then until while false true goto",
    "assert collectgarbage dofile error _G "
    "getmetatable ipairs loadfile next pairs "
    "pcall print rawequal rawget rawset "
    "setmetatable tonumber tostring type _VERSION "
    "xpcall string table math coroutine io os debug "
    "getfenv gcinfo load loadlib loadstring "
    "require select setfenv unpack "
    "_LOADED LUA_PATH _REQUIREDNAME "
    "package rawlen package bit32 utf8 _ENV",
    "string.byte string.char string.dump string.find string.format "
    "string.gsub string.len string.lower string.rep string.sub string.upper "
    "table.concat table.insert table.remove table.sort "
    "math.abs math.acos math.asin math.atan math.atan2 "
    "math.ceil math.cos math.deg math.exp math.floor "
    "math.frexp math.ldexp math.log math.max math.min "
    "math.pi math.pow math.rad math.random math.randomseed "
    "math.sin math.sqrt math.tan "
    "string.gfind string.gmatch string.match string.reverse "
    "string.pack string.packsize string.unpack "
    "table.foreach table.foreachi table.getn table.setn "
    "table.maxn table.pack table.unpack table.move "
    "math.cosh math.fmod math.huge math.log10 math.modf "
    "math.mod math.sinh math.tanh math.maxinteger math.mininteger "
    "math.tointeger math.type math.ult "
    "bit32.arshift bit32.band bit32.bnot bit32.bor bit32.btest "
    "bit32.bxor bit32.extract bit32.replace bit32.lrotate bit32.lshift "
    "bit32.rrotate bit32.rshift "
    "utf8.char utf8.charpattern utf8.codes "
    "utf8.codepoint utf8.len utf8.offset",
    "coroutine.create coroutine.resume coroutine.status coroutine.wrap coroutine.yield "
    "io.close io.flush io.input io.lines io.open "
    "io.output io.read io.tmpfile io.type io.write "
    "io.stdin io.stdout io.stderr "
    "os.clock os.date os.difftime os.execute os.exit "
    "os.getenv os.remove os.rename os.setlocale os.time "
    "os.tmpname "
    "coroutine.isyieldable coroutine.running io.popen "
    "module package.loaders package.seeall "
    "package.config package.searchers package.searchpath "
    "require package.cpath package.loaded "
    "package.loadlib package.path package.preload",
    "", "", "", "", ""
};


EDITLEXER lexLua = { SCLEX_LUA, 63029, L"Lua", L"lua", L"", &KeyWords_Lua, COMMENT_INFO(LUA_COMMENT, SCE_LUA_COMMENTLINE), {
        { SCE_LUA_DEFAULT, 63126, L"Default", L"fore:#FF0000", L"" },
        { SCE_LUA_COMMENT, 63127, L"Comment", L"fore:#007F00,back:#D0F0F0,eolfilled", L"" },
        { SCE_LUA_COMMENTLINE, 63270, L"Comment Line", L"fore:#007F00", L"" },
        { SCE_LUA_COMMENTDOC, 63271, L"Comment Doc", L"back:#FF0000", L"" },
        { SCE_LUA_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_LUA_WORD, 63128, L"Keyword", L"fore:#00007F", L"" },
        { SCE_LUA_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_LUA_CHARACTER, 63265, L"Char", L"fore:#7F007F", L"" },
        { SCE_LUA_LITERALSTRING, 63295, L"Literal String", L"fore:#7F007F,back:#E0FFFF", L"" },
        { SCE_LUA_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#7F7F00", L"" },
        { SCE_LUA_OPERATOR, 63132, L"Operator", L"fore:#000000", L"" },
        { SCE_LUA_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_LUA_STRINGEOL, 63274, L"String EOL", L"back:#E0C0E0,eolfilled", L"" },
        { SCE_LUA_WORD2, 63262, L"Keyword 2", L"fore:#00007F,back:#F5FFF5", L"" },
        { SCE_LUA_WORD3, 63263, L"Keyword 3", L"fore:#00007F,back:#F5F5FF", L"" },
        { SCE_LUA_WORD4, 63289, L"Keyword 4", L"fore:#00007F,back:#FFF5F5", L"" },
        { SCE_LUA_WORD5, 63290, L"Keyword 5", L"fore:#00007F,back:#FFF5FF", L"" },
        { SCE_LUA_WORD6, 63291, L"Keyword 6", L"fore:#00007F,back:#FFFFF5", L"" },
        { SCE_LUA_WORD7, 63292, L"Keyword 7", L"fore:#00007F,back:#FFA0A0", L"" },
        { SCE_LUA_WORD8, 63293, L"Keyword 8", L"fore:#00007F,back:#FFF5F5", L"" },
        { SCE_LUA_LABEL, 63235, L"Label", L"fore:#7F7F00", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_NSIS = {
    "!addincludedir !addplugindir MakeNSIS Portions "
    "Contributors: Abort AddBrandingImage AddSize AutoCloseWindow BGFont "
    "BGGradient BrandingText BringToFront Call CallInstDLL Caption ChangeUI "
    "ClearErrors ComponentText GetDLLVersion GetDLLVersionLocal GetFileTime "
    "GetFileTimeLocal CopyFiles CRCCheck CreateDirectory CreateFont CreateShortCut "
    "SetDatablockOptimize DeleteINISec DeleteINIStr DeleteRegKey DeleteRegValue "
    "Delete DetailPrint DirText DirShow DirVar DirVerify GetInstDirError AllowRootDirInstall "
    "CheckBitmap EnableWindow EnumRegKey EnumRegValue Exch Exec ExecWait "
    "ExecShell ExpandEnvStrings FindWindow FindClose FindFirst FindNext File FileBufSize "
    "FlushINI ReserveFile FileClose FileErrorText FileOpen FileRead FileWrite FileReadByte "
    "FileWriteByte FileSeek Function FunctionEnd GetDlgItem GetFullPathName "
    "GetTempFileName HideWindow Icon IfAbort IfErrors IfFileExists IfRebootFlag IfSilent "
    "InstallDirRegKey InstallColors InstallDir InstProgressFlags InstType IntOp IntCmp "
    "IntCmpU IntFmt IsWindow Goto LangString LangStringUP LicenseData "
    "LicenseForceSelection LicenseLangString LicenseText LicenseBkColor "
    "LoadLanguageFile LogSet LogText MessageBox Nop Name OutFile Page "
    "PageCallbacks PageEx PageExEnd Pop Push Quit ReadINIStr ReadRegDWORD "
    "ReadRegStr ReadEnvStr Reboot RegDLL Rename Return RMDir Section SectionEnd "
    "SectionIn SubSection SectionGroup SubSectionEnd SectionGroupEnd SearchPath "
    "SectionSetFlags SectionGetFlags SectionSetInstTypes SectionGetInstTypes "
    "SectionGetText SectionSetText SectionGetSize SectionSetSize GetCurInstType "
    "SetCurInstType InstTypeSetText InstTypeGetText SendMessage SetAutoClose "
    "SetCtlColors SetBrandingImage SetCompress SetCompressor SetCompressorDictSize "
    "SetCompressionLevel SetDateSave SetDetailsView SetDetailsPrint SetErrors "
    "SetErrorLevel GetErrorLevel SetFileAttributes SetFont SetOutPath SetOverwrite "
    "SetPluginUnload SetRebootFlag SetShellVarContext SetSilent ShowInstDetails "
    "ShowUninstDetails ShowWindow SilentInstall SilentUnInstall Sleep StrCmp StrCpy "
    "StrLen SubCaption UninstallExeName UninstallCaption UninstallIcon UninstPage "
    "UninstallText UninstallSubCaption UnRegDLL WindowIcon WriteINIStr WriteRegBin "
    "WriteRegDWORD WriteRegStr WriteRegExpandStr WriteUninstaller XPStyle !packhdr "
    "!system !execute !AddIncludeDir !include !cd !ifdef !ifndef !endif !define !undef !else !echo !warning "
    "!error !verbose !macro !macroend !insertmacro !ifmacrodef !ifmacrondef MiscButtonText "
    "DetailsButtonText UninstallButtonText InstallButtonText SpaceTexts "
    "CompletedText GetFunctionAddress GetLabelAddress GetCurrentAddress "
    "!AddPluginDir InitPluginsDir AllowSkipFiles Var VIAddVersionKey VIProductVersion LockWindow",
    "$0 $1 $2 $3 $4 $5 $6 $7 $8 $9 "
    "$R0 $R1 $R2 $R3 $R4 $R5 $R6 $R7 $R8 $R9 $\\t $\\\" $\\' $\\` "
    "$VARNAME $0, $INSTDIR $OUTDIR $CMDLINE $LANGUAGE $PROGRAMFILES "
    "$COMMONFILES $DESKTOP $EXEDIR ${NSISDIR} $WINDIR $SYSDIR $TEMP "
    "$STARTMENU $SMPROGRAMS $SMSTARTUP $QUICKLAUNCH $DOCUMENTS "
    "$SENDTO $RECENT $FAVORITES $MUSIC $PICTURES $VIDEOS $NETHOOD "
    "$FONTS $TEMPLATES $APPDATA $PRINTHOOD $INTERNET_CACHE $COOKIES "
    "$HISTORY $PROFILE $ADMINTOOLS $RESOURCES $RESOURCES_LOCALIZED "
    "$CDBURN_AREA $HWNDPARENT $PLUGINSDIR $$ $\\r $\\n",
    "ARCHIVE FILE_ATTRIBUTE_ARCHIVE FILE_ATTRIBUTE_HIDDEN "
    "FILE_ATTRIBUTE_NORMAL FILE_ATTRIBUTE_OFFLINE FILE_ATTRIBUTE_READONLY "
    "FILE_ATTRIBUTE_SYSTEM FILE_ATTRIBUTE_TEMPORARY HIDDEN HKCC HKCR HKCU "
    "HKDD HKEY_CLASSES_ROOT HKEY_CURRENT_CONFIG HKEY_CURRENT_USER HKEY_DYN_DATA "
    "HKEY_LOCAL_MACHINE HKEY_PERFORMANCE_DATA HKEY_USERS HKLM HKPD HKU IDABORT "
    "IDCANCEL IDIGNORE IDNO IDOK IDRETRY IDYES MB_ABORTRETRYIGNORE MB_DEFBUTTON1 "
    "MB_DEFBUTTON2 MB_DEFBUTTON3 MB_DEFBUTTON4 MB_ICONEXCLAMATION "
    "MB_ICONINFORMATION MB_ICONQUESTION MB_ICONSTOP MB_OK MB_OKCANCEL "
    "MB_RETRYCANCEL MB_RIGHT MB_SETFOREGROUND MB_TOPMOST MB_YESNO "
    "MB_YESNOCANCEL NORMAL OFFLINE READONLY SW_SHOWMAXIMIZED SW_SHOWMINIMIZED "
    "SW_SHOWNORMAL SYSTEM TEMPORARY auto colored false force hide ifnewer nevershow "
    "normal off on show silent silentlog smooth true try lzma zlib bzip2 none listonly textonly "
    "both top left bottom right license components directory instfiles uninstConfirm custom "
    "all leave current ifdiff lastused LEFT RIGHT CENTER dlg_id ALT CONTROL EXT SHIFT "
    "open print manual alwaysoff",
    "", "", "", "", "", ""
};


EDITLEXER lexNSIS = { SCLEX_NSIS, 63030, L"NSIS Script", L"nsi;nsh", L"", &KeyWords_NSIS, COMMENT_INFO(BASH_COMMENT, SCE_NSIS_COMMENT), {
        { SCE_NSIS_DEFAULT, 63126, L"Default", L"fore:#000000", L"" },
        { SCE_NSIS_COMMENT, 63127, L"Comment", L"fore:#007F00", L"" },
        { SCE_NSIS_STRINGDQ, 63211, L"String double quoted", L"fore:#999999,back:#EEEEEE", L"" },
        { SCE_NSIS_STRINGLQ, 63298, L"String left quote", L"fore:#999999,back:#EEEEEE", L"" },
        { SCE_NSIS_STRINGRQ, 63299, L"String right quote", L"fore:#999999,back:#EEEEEE", L"" },
        { SCE_NSIS_FUNCTION, 63247, L"Function name", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_VARIABLE, 63249, L"Variable", L"fore:#CC3300", L"" },
        { SCE_NSIS_LABEL, 63235, L"Label", L"fore:#FF9900", L"" },
        { SCE_NSIS_USERDEFINED, 63300, L"User defined", L"fore:#000000", L"" },
        { SCE_NSIS_SECTIONDEF, 63232, L"Section", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_SUBSECTIONDEF, 63301, L"Subsection", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_IFDEFINEDEF, 63302, L"If def", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_MACRODEF, 63303, L"Macro def", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_STRINGVAR, 63304, L"Variable within string", L"fore:#CC3300,back:#EEEEEE", L"" },
        { SCE_NSIS_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_NSIS_SECTIONGROUP, 63305, L"Section Group", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_PAGEEX, 63306, L"Page Ex", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_FUNCTIONDEF, 63307, L"Function Definition", L"fore:#00007F,bold", L"" },
        { SCE_NSIS_COMMENTBOX, 63308, L"Comment Box", L"fore:#007F00,bold", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_TeX = {
    "above abovedisplayshortskip abovedisplayskip "
    "abovewithdelims accent adjdemerits advance afterassignment "
    "aftergroup atop atopwithdelims "
    "badness baselineskip batchmode begingroup "
    "belowdisplayshortskip belowdisplayskip binoppenalty botmark "
    "box boxmaxdepth brokenpenalty "
    "catcode char chardef cleaders closein closeout clubpenalty "
    "copy count countdef cr crcr csname "
    "day deadcycles def defaulthyphenchar defaultskewchar "
    "delcode delimiter delimiterfactor delimeters "
    "delimitershortfall delimeters dimen dimendef discretionary "
    "displayindent displaylimits displaystyle "
    "displaywidowpenalty displaywidth divide "
    "doublehyphendemerits dp dump "
    "edef else emergencystretch end endcsname endgroup endinput "
    "endlinechar eqno errhelp errmessage errorcontextlines "
    "errorstopmode escapechar everycr everydisplay everyhbox "
    "everyjob everymath everypar everyvbox exhyphenpenalty "
    "expandafter "
    "fam fi finalhyphendemerits firstmark floatingpenalty font "
    "fontdimen fontname futurelet "
    "gdef global group globaldefs "
    "halign hangafter hangindent hbadness hbox hfil horizontal "
    "hfill horizontal hfilneg hfuzz hoffset holdinginserts hrule "
    "hsize hskip hss horizontal ht hyphenation hyphenchar "
    "hyphenpenalty hyphen "
    "if ifcase ifcat ifdim ifeof iffalse ifhbox ifhmode ifinner "
    "ifmmode ifnum ifodd iftrue ifvbox ifvmode ifvoid ifx "
    "ignorespaces immediate indent input inputlineno "
    "insert insertpenalties interlinepenalty "
    "jobname "
    "kern "
    "language lastbox lastkern lastpenalty lastskip lccode "
    "leaders left lefthyphenmin leftskip leqno let limits "
    "linepenalty line lineskip lineskiplimit long looseness "
    "lower lowercase "
    "mag mark mathaccent mathbin mathchar mathchardef mathchoice "
    "mathclose mathcode mathinner mathop mathopen mathord "
    "mathpunct mathrel mathsurround maxdeadcycles maxdepth "
    "meaning medmuskip message mkern month moveleft moveright "
    "mskip multiply muskip muskipdef "
    "newlinechar noalign noboundary noexpand noindent nolimits "
    "nonscript scriptscript nonstopmode nulldelimiterspace "
    "nullfont number "
    "omit openin openout or outer output outputpenalty over "
    "overfullrule overline overwithdelims "
    "pagedepth pagefilllstretch pagefillstretch pagefilstretch "
    "pagegoal pageshrink pagestretch pagetotal par parfillskip "
    "parindent parshape parskip patterns pausing penalty "
    "postdisplaypenalty predisplaypenalty predisplaysize "
    "pretolerance prevdepth prevgraf "
    "radical raise read relax relpenalty right righthyphenmin "
    "rightskip romannumeral "
    "scriptfont scriptscriptfont scriptscriptstyle scriptspace "
    "scriptstyle scrollmode setbox setlanguage sfcode shipout "
    "show showbox showboxbreadth showboxdepth showlists showthe "
    "skewchar skip skipdef spacefactor spaceskip span special "
    "splitbotmark splitfirstmark splitmaxdepth splittopskip "
    "string "
    "tabskip textfont textstyle the thickmuskip thinmuskip time "
    "toks toksdef tolerance topmark topskip tracingcommands "
    "tracinglostchars tracingmacros tracingonline tracingoutput "
    "tracingpages tracingparagraphs tracingrestores tracingstats "
    "uccode uchyph underline unhbox unhcopy unkern unpenalty "
    "unskip unvbox unvcopy uppercase "
    "vadjust valign vbadness vbox vcenter vfil vfill vfilneg "
    "vfuzz voffset vrule vsize vskip vsplit vss vtop "
    "wd widowpenalty write "
    "xdef xleaders xspaceskip "
    "year "
    "TeX "
    "bgroup egroup endgraf space empty null "
    "newcount newdimen newskip newmuskip newbox newtoks newhelp newread newwrite newfam newlanguage newinsert newif "
    "maxdimen magstephalf magstep "
    "frenchspacing nonfrenchspacing normalbaselines obeylines obeyspaces raggedright ttraggedright "
    "thinspace negthinspace enspace enskip quad qquad "
    "smallskip medskip bigskip removelastskip topglue vglue hglue "
    "break nobreak allowbreak filbreak goodbreak smallbreak medbreak bigbreak "
    "line leftline rightline centerline rlap llap underbar strutbox strut "
    "cases matrix pmatrix bordermatrix eqalign displaylines eqalignno leqalignno "
    "pageno folio tracingall showhyphens fmtname fmtversion "
    "hphantom vphantom phantom smash",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexTeX = { SCLEX_TEX, 63031, L"TeX", L"tex;sty", L"", &KeyWords_TeX, NULL_COMMENT_INFO, {
        { SCE_TEX_DEFAULT, 63126, L"Default", L"fore:#3F3F3F", L"" },
        { SCE_TEX_SPECIAL, 63295, L"Special", L"fore:#007F7F", L"" },
        { SCE_TEX_GROUP, 63321, L"Group", L"fore:#7F0000", L"" },
        { SCE_TEX_SYMBOL, 63316, L"Symbol", L"fore:#7F7F00", L"" },
        { SCE_TEX_COMMAND, 63236, L"Command", L"fore:#007F00", L"" },
        { SCE_TEX_TEXT, 63322, L"Text", L"", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_Yaml = {
    "true false yes no",
    "", "", "", "", "", "", "", ""
};


EDITLEXER lexYaml = { SCLEX_YAML, 63032, L"YAML", L"yaml;yml", L"", &KeyWords_Yaml, COMMENT_INFO(BASH_COMMENT, SCE_YAML_COMMENT), {
        { SCE_YAML_DEFAULT, 63126, L"Default", L"fore:#000000", L"" },
        { SCE_YAML_COMMENT, 63127, L"Comment", L"fore:#008800", L"" },
        { SCE_YAML_IDENTIFIER, 63129, L"Identifier", L"fore:#000088,bold", L"" },
        { SCE_YAML_KEYWORD, 63128, L"Keyword", L"fore:#880088", L"" },
        { SCE_YAML_NUMBER, 63130, L"Number", L"fore:#880000", L"" },
        { SCE_YAML_REFERENCE, 63323, L"Reference", L"fore:#008888", L"" },
        { SCE_YAML_DOCUMENT, 63324, L"Document", L"fore:#FFFFFF,bold,back:#000088,eolfilled", L"" },
        { SCE_YAML_TEXT, 63322, L"Text", L"fore:#333366", L"" },
        { SCE_YAML_ERROR, 63325, L"Error", L"fore:#FFFFFF,italics,bold,back:#FF0000,eolfilled", L"" },
        { SCE_YAML_OPERATOR, 63132, L"Operator", L"fore:#000000", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_Rust = {
  "alignof as be box break const continue crate do else enum extern false fn "
  "for if impl in let loop match mod mut offsetof once priv proc pub pure ref "
  "return self sizeof static struct super trait true type typeof unsafe unsized use virtual while yield",
  "bool char f32 f64 i16 i32 i64 i8 int str u16 u32 u64 u8 uint",
  "Self",
  "", "", "", "", "", ""
};


EDITLEXER lexRust = { SCLEX_RUST, 63033, L"Rust", L"rs", L"", &KeyWords_Rust, COMMENT_INFO(C_COMMENT, SCE_RUST_COMMENTLINE), {
        { SCE_RUST_DEFAULT, 63126, L"Default", L"fore:#808080", L"" },
        { SCE_RUST_COMMENTBLOCK, 63282, L"Comment Block", L"fore:#007F00", L"" },
        { SCE_RUST_COMMENTLINE, 63270, L"Comment Line", L"fore:#007F00", L"" },
        { SCE_RUST_COMMENTBLOCKDOC, 63326, L"Comment Block Dock", L"fore:#3F703F", L"" },
        { SCE_RUST_COMMENTLINEDOC, 63276, L"Comment Line Doc", L"fore:#3F703F", L"" },
        { SCE_RUST_NUMBER, 63130, L"Number", L"fore:#007F7F", L"" },
        { SCE_RUST_WORD, 63128, L"Keyword", L"fore:#00007F,bold", L"" },
        { SCE_RUST_WORD2, 63262, L"Keyword 2", L"fore:#00007F,bold", L"" },
        { SCE_RUST_WORD3, 63263, L"Keyword 3", L"fore:#00007F", L"" },
        { SCE_RUST_WORD4, 63289, L"Keyword 4", L"fore:#00007F,bold", L"" },
        { SCE_RUST_WORD5, 63290, L"Keyword 5", L"fore:#00007F,bold", L"" },
        { SCE_RUST_WORD6, 63291, L"Keyword 6", L"fore:#00007F,bold", L"" },
        { SCE_RUST_WORD7, 63292, L"Keyword 7", L"fore:#00007F,bold", L"" },
        { SCE_RUST_STRING, 63131, L"String", L"fore:#7F007F", L"" },
        { SCE_RUST_STRINGR, 63288, L"String R", L"fore:#B090B0", L"" },
        { SCE_RUST_CHARACTER, 63265, L"Char", L"fore:#7F007F", L"" },
        { SCE_RUST_OPERATOR, 63132, L"Operator", L"fore:#000000,bold", L"" },
        { SCE_RUST_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { SCE_RUST_LIFETIME, 63327, L"Lifetime", L"fore:#007F7F", L"" },
        { SCE_RUST_MACRO, 63303, L"Macro def", L"fore:#7F7F00", L"" },
        { SCE_RUST_LEXERROR, 63325, L"Error", L"fore:#000000,back:#E0C0E0", L"" },
        { SCE_RUST_BYTESTRING, 63328, L"Byte String", L"fore:#7F007F", L"" },
        { SCE_RUST_BYTESTRINGR, 63329, L"Byte String R", L"fore:#B090B0", L"" },
        { SCE_RUST_BYTECHARACTER, 63330, L"Byte Char", L"fore:#7F007F", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


KEYWORDLIST KeyWords_Markdown = {
    "", "", "", "", "", "", "", "", ""
};


EDITLEXER lexMarkdown = { SCLEX_MARKDOWN, 63034, L"Markdown", L"markdown;mdown;mkdn;md;mkd;mdwn;mdtxt;mdtext", L"", &KeyWords_Markdown, NULL_COMMENT_INFO, {
        { SCE_MARKDOWN_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_MARKDOWN_LINE_BEGIN, 63331, L"Line Begin", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_STRONG1, 63332, L"Strong", L"fore:#DDBB99,bold", L"" },
        { SCE_MARKDOWN_STRONG2, 63333, L"Strong 2", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_EM1, 63334, L"EM1", L"fore:#CC00FF,italics", L"" },
        { SCE_MARKDOWN_EM2, 63335, L"EM2", L"fore:#9999FF,italics", L"" },
        { SCE_MARKDOWN_HEADER1, 63336, L"Header 1", L"fore:#0088FF,bold", L"" },
        { SCE_MARKDOWN_HEADER2, 63337, L"Header 2", L"fore:#2299FF,bold", L"" },
        { SCE_MARKDOWN_HEADER3, 63338, L"Header 3", L"fore:#44AAFF,bold", L"" },
        { SCE_MARKDOWN_HEADER4, 63339, L"Header 4", L"fore:#CC00FF,bold", L"" },
        { SCE_MARKDOWN_HEADER5, 63340, L"Header 5", L"fore:#CC00FF,bold", L"" },
        { SCE_MARKDOWN_HEADER6, 63341, L"Header 6", L"fore:#CC00FF,bold", L"" },
        { SCE_MARKDOWN_PRECHAR, 63342, L"Prechar", L"fore:#000000", L"" },
        { SCE_MARKDOWN_ULIST_ITEM, 63343, L"UList Item", L"fore:#CC33FF", L"" },
        { SCE_MARKDOWN_OLIST_ITEM, 63344, L"OList Item", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_BLOCKQUOTE, 63345, L"Block Quote", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_STRIKEOUT, 63346, L"Strike Out", L"", L"" },
        { SCE_MARKDOWN_HRULE, 63347, L"HRule", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_LINK, 63348, L"Link", L"fore:#FF5555", L"" },
        { SCE_MARKDOWN_CODE, 63349, L"Code", L"fore:#80FF80,back:#100010", L"" },
        { SCE_MARKDOWN_CODE2, 63350, L"Code 2", L"fore:#CC00FF", L"" },
        { SCE_MARKDOWN_CODEBK, 63351, L"Code Block", L"fore:#CC00FF", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};


// [2e]: Awk syntax highlighting #216
// Original lexer implementation: https://github.com/rizonesoft/Notepad3/blob/master/src/StyleLexers/styleLexAwk.c
KEYWORDLIST KeyWords_Awk = {
    "break case continue default do else exit function for if in next return switch while "
    "@include delete nextfile print printf BEGIN BEGINFILE END "
    "atan2 cos exp int log rand sin sqrt srand asort asorti gensub gsub index "
    "length match patsplit split sprintf strtonum sub substr tolower toupper close "
    "fflush system mktime strftime systime and compl lshift rshift xor "
    "isarray bindtextdomain dcgettext dcngettext",
    "ARGC ARGIND ARGV FILENAME FNR FS NF NR OFMT OFS ORS RLENGTH RS RSTART SUBSEP TEXTDOMAIN "
    "BINMODE CONVFMT FIELDWIDTHS FPAT IGNORECASE LINT TEXTDOMAiN ENVIRON ERRNO PROCINFO RT",
    "", "", "", "", "", ""
};


EDITLEXER lexAWK = { SCLEX_PYTHON, 63035, L"Awk Script", L"awk", L"", &KeyWords_Awk, COMMENT_INFO(BASH_COMMENT, SCE_P_COMMENTLINE), {
        { STYLE_DEFAULT, 63126, L"Default", L"", L"" },
        { SCE_P_WORD, 63128, L"Keyword", L"bold; fore:#0000A0", L"" },
        { SCE_P_WORD, 63262, L"Keyword 2", L"bold; italic; fore:#6666FF", L"" },
        { SCE_P_IDENTIFIER, 63129, L"Identifier", L"", L"" },
        { MULTI_STYLE(SCE_P_COMMENTLINE,SCE_P_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#808080", L"" },
        { MULTI_STYLE(SCE_P_STRING,SCE_P_STRINGEOL,SCE_P_CHARACTER,0), 63131, L"String", L"fore:#008000", L"" },
        { SCE_P_NUMBER, 63130, L"Number", L"fore:#C04000", L"" },
        { SCE_P_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};
// [/2e]


// [2e]: AHK syntax highlighting #214
// Original lexer implementation: https://github.com/XhmikosR/notepad2-mod/blob/master/scintilla/lexers/LexAHK.cxx
KEYWORDLIST KeyWords_AHK = {
    "break continue else exit exitapp gosub goto if ifequal ifexist ifgreater ifgreaterorequal "
    "ifinstring ifless iflessorequal ifmsgbox ifnotequal ifnotexist ifnotinstring ifwinactive "
    "ifwinexist ifwinnotactive ifwinnotexist loop onexit pause repeat return settimer sleep "
    "suspend static global local var byref while until for class try catch throw",
    "autotrim blockinput clipwait control controlclick controlfocus controlget controlgetfocus "
    "controlgetpos controlgettext controlmove controlsend controlsendraw controlsettext coordmode "
    "critical detecthiddentext detecthiddenwindows drive driveget drivespacefree edit endrepeat "
    "envadd envdiv envget envmult envset envsub envupdate fileappend filecopy filecopydir filecreatedir "
    "filecreateshortcut filedelete filegetattrib filegetshortcut filegetsize filegettime filegetversion "
    "fileinstall filemove filemovedir fileread filereadline filerecycle filerecycleempty fileremovedir "
    "fileselectfile fileselectfolder filesetattrib filesettime formattime getkeystate groupactivate "
    "groupadd groupclose groupdeactivategui guicontrol guicontrolget hideautoitwin hotkey imagesearch "
    "inidelete iniread iniwrite input inputbox keyhistory keywait listhotkeys listlines listvars menu "
    "mouseclick mouseclickdrag mousegetpos mousemove msgbox outputdebug pixelgetcolor pixelsearch "
    "postmessage process progress random regdelete regread regwrite reload run runas runwait send "
    "sendevent sendinput sendmessage sendmode sendplay sendraw setbatchlines setcapslockstate "
    "setcontroldelay setdefaultmousespeed setenv setformat setkeydelay setmousedelay setnumlockstate "
    "setscrolllockstate setstorecapslockmode settitlematchmode setwindelay setworkingdir shutdown sort "
    "soundbeep soundget soundgetwavevolume soundplay soundset soundsetwavevolume splashimage splashtextoff "
    "splashtexton splitpath statusbargettext statusbarwait stringcasesense stringgetpos stringleft stringlen "
    "stringlower stringmid stringreplace stringright stringsplit stringtrimleft stringtrimright stringupper "
    "sysget thread tooltip transform traytip urldownloadtofile winactivate winactivatebottom winclose winget "
    "wingetactivestats wingetactivetitle wingetclass wingetpos wingettext wingettitle winhide winkill "
    "winmaximize winmenuselectitem winminimize winminimizeall winminimizeallundo winmove winrestore winset "
    "winsettitle winshow winwait winwaitactive winwaitclose winwaitnotactive fileencoding",
    "abs acos asc asin atan ceil chr cos dllcall exp fileexist floor getkeystate numget numput "
    "registercallback il_add il_create il_destroy instr islabel isfunc ln log lv_add lv_delete "
    "lv_deletecol lv_getcount lv_getnext lv_gettext lv_insert lv_insertcol lv_modify lv_modifycol "
    "lv_setimagelist mod onmessage round regexmatch regexreplace sb_seticon sb_setparts sb_settext "
    "sin sqrt strlen substr tan tv_add tv_delete tv_getchild tv_getcount tv_getnext tv_get tv_getparent "
    "tv_getprev tv_getselection tv_gettext tv_modify tv_setimagelist varsetcapacity winactive winexist "
    "trim ltrim rtrim fileopen strget strput object array isobject objinsert objremove objminindex "
    "objmaxindex objsetcapacity objgetcapacity objgetaddress objnewenum objaddref objrelease objhaskey "
    "objclone _insert _remove _minindex _maxindex _setcapacity _getcapacity _getaddress _newenum _addref "
    "_release _haskey _clone comobjcreate comobjget comobjconnect comobjerror comobjactive comobjenwrap "
    "comobjunwrap comobjparameter comobjmissing comobjtype comobjvalue comobjarray comobjquery comobjflags "
    "func getkeyname getkeyvk getkeysc isbyref exception",
    "allowsamelinecomments clipboardtimeout commentflag errorstdout escapechar hotkeyinterval "
    "hotkeymodifiertimeout hotstring if iftimeout ifwinactive ifwinexist include includeagain "
    "installkeybdhook installmousehook keyhistory ltrim maxhotkeysperinterval maxmem maxthreads "
    "maxthreadsbuffer maxthreadsperhotkey menumaskkey noenv notrayicon persistent singleinstance "
    "usehook warn winactivateforce",
    "shift lshift rshift alt lalt ralt control lcontrol rcontrol ctrl lctrl rctrl lwin rwin appskey "
    "altdown altup shiftdown shiftup ctrldown ctrlup lwindown lwinup rwindown rwinup lbutton rbutton "
    "mbutton wheelup wheeldown xbutton1 xbutton2 joy1 joy2 joy3 joy4 joy5 joy6 joy7 joy8 joy9 joy10 "
    "joy11 joy12 joy13 joy14 joy15 joy16 joy17 joy18 joy19 joy20 joy21 joy22 joy23 joy24 joy25 joy26 "
    "joy27 joy28 joy29 joy30 joy31 joy32 joyx joyy joyz joyr joyu joyv joypov joyname joybuttons "
    "joyaxes joyinfo space tab enter escape esc backspace bs delete del insert ins pgup pgdn home end "
    "up down left right printscreen ctrlbreak pause scrolllock capslock numlock numpad0 numpad1 numpad2 "
    "numpad3 numpad4 numpad5 numpad6 numpad7 numpad8 numpad9 numpadmult numpadadd numpadsub numpaddiv "
    "numpaddot numpaddel numpadins numpadclear numpadup numpaddown numpadleft numpadright numpadhome "
    "numpadend numpadpgup numpadpgdn numpadenter f1 f2 f3 f4 f5 f6 f7 f8 f9 f10 f11 f12 f13 f14 f15 "
    "f16 f17 f18 f19 f20 f21 f22 f23 f24 browser_back browser_forward browser_refresh browser_stop "
    "browser_search browser_favorites browser_home volume_mute volume_down volume_up media_next "
    "media_prev media_stop media_play_pause launch_mail launch_media launch_app1 launch_app2 blind "
    "click raw wheelleft wheelright",
    "a_ahkpath a_ahkversion a_appdata a_appdatacommon a_autotrim a_batchlines a_caretx a_carety "
    "a_computername a_controldelay a_cursor a_dd a_ddd a_dddd a_defaultmousespeed a_desktop "
    "a_desktopcommon a_detecthiddentext a_detecthiddenwindows a_endchar a_eventinfo a_exitreason "
    "a_formatfloat a_formatinteger a_gui a_guievent a_guicontrol a_guicontrolevent a_guiheight "
    "a_guiwidth a_guix a_guiy a_hour a_iconfile a_iconhidden a_iconnumber a_icontip a_index "
    "a_ipaddress1 a_ipaddress2 a_ipaddress3 a_ipaddress4 a_isadmin a_iscompiled a_issuspended "
    "a_keydelay a_language a_lasterror a_linefile a_linenumber a_loopfield a_loopfileattrib "
    "a_loopfiledir a_loopfileext a_loopfilefullpath a_loopfilelongpath a_loopfilename "
    "a_loopfileshortname a_loopfileshortpath a_loopfilesize a_loopfilesizekb a_loopfilesizemb "
    "a_loopfiletimeaccessed a_loopfiletimecreated a_loopfiletimemodified a_loopreadline a_loopregkey "
    "a_loopregname a_loopregsubkey a_loopregtimemodified a_loopregtype a_mday a_min a_mm a_mmm "
    "a_mmmm a_mon a_mousedelay a_msec a_mydocuments a_now a_nowutc a_numbatchlines a_ostype "
    "a_osversion a_priorhotkey a_programfiles a_programs a_programscommon a_screenheight "
    "a_screenwidth a_scriptdir a_scriptfullpath a_scriptname a_sec a_space a_startmenu "
    "a_startmenucommon a_startup a_startupcommon a_stringcasesense a_tab a_temp a_thishotkey "
    "a_thismenu a_thismenuitem a_thismenuitempos a_tickcount a_timeidle a_timeidlephysical "
    "a_timesincepriorhotkey a_timesincethishotkey a_titlematchmode a_titlematchmodespeed "
    "a_username a_wday a_windelay a_windir a_workingdir a_yday a_year a_yweek a_yyyy "
    "clipboard clipboardall comspec errorlevel programfiles true false a_thisfunc a_thislabel "
    "a_ispaused a_iscritical a_isunicode a_ptrsize a_scripthwnd a_priorkey",
    "ltrim rtrim join ahk_id ahk_pid ahk_class ahk_group ahk_exe processname processpath minmax "
    "controllist statuscd filesystem setlabel alwaysontop mainwindow nomainwindow useerrorlevel "
    "altsubmit hscroll vscroll imagelist wantctrla wantf2 vis visfirst wantreturn backgroundtrans "
    "minimizebox maximizebox sysmenu toolwindow exstyle check3 checkedgray readonly notab lastfound "
    "lastfoundexist alttab shiftalttab alttabmenu alttabandmenu alttabmenudismiss controllisthwnd "
    "hwnd deref pow bitnot bitand bitor bitxor bitshiftleft bitshiftright sendandmouse mousemove "
    "mousemoveoff hkey_local_machine hkey_users hkey_current_user hkey_classes_root hkey_current_config "
    "hklm hku hkcu hkcr hkcc reg_sz reg_expand_sz reg_multi_sz reg_dword reg_qword reg_binary reg_link "
    "reg_resource_list reg_full_resource_descriptor reg_resource_requirements_list reg_dword_big_endian "
    "regex pixel mouse screen relative rgb low belownormal normal abovenormal high realtime between "
    "contains in is integer float number digit xdigit integerfast floatfast alpha upper lower alnum "
    "time date not or and topmost top bottom transparent transcolor redraw region id idlast count "
    "list capacity eject lock unlock label serial type status seconds minutes hours days read parse "
    "logoff close error single shutdown menu exit reload tray add rename check uncheck togglecheck "
    "enable disable toggleenable default nodefault standard nostandard color delete deleteall icon "
    "noicon tip click show edit progress hotkey text picture pic groupbox button checkbox radio "
    "dropdownlist ddl combobox statusbar treeview listbox listview datetime monthcal updown slider "
    "tab tab2 activex iconsmall tile report sortdesc nosort nosorthdr grid hdr autosize range xm ym "
    "ys xs xp yp font resize owner submit nohide minimize maximize restore noactivate na cancel "
    "destroy center margin owndialogs guiescape guiclose guisize guicontextmenu guidropfiles tabstop "
    "section wrap border top bottom buttons expand first lines number uppercase lowercase limit "
    "password multi group background bold italic strike underline norm theme caption delimiter flash "
    "style checked password hidden left right center section move focus hide choose choosestring text "
    "pos enabled disabled visible notimers interrupt priority waitclose unicode tocodepage fromcodepage "
    "yes no ok cancel abort retry ignore force on off all send wanttab monitorcount monitorprimary "
    "monitorname monitorworkarea pid this base extends __get __set __call __delete __new new "
    "useunsetlocal useunsetglobal useenv localsameasglobal",
    "", ""
};


EDITLEXER lexAHK = { SCLEX_AHK, 63036, L"AutoHotkey Script", L"ahk; ia; scriptlet", L"", &KeyWords_AHK, COMMENT_INFO(ASM_COMMENT, SCE_AHK_COMMENTLINE), {
        { SCE_AHK_DEFAULT, 63126, L"Default", L"", L"" },
        { MULTI_STYLE(SCE_AHK_COMMENTLINE,SCE_AHK_COMMENTBLOCK,0,0), 63127, L"Comment", L"fore:#008000", L"" },
        { SCE_AHK_ESCAPE, 63352, L"Escape", L"fore:#FF8000", L"" },
        { SCE_AHK_SYNOPERATOR, 63353, L"Syntax operator", L"fore:#7F200F", L"" },
        { SCE_AHK_EXPOPERATOR, 63354, L"Expression operator", L"fore:#FF4F00", L"" },
        { SCE_AHK_STRING, 63131, L"String", L"fore:#404040", L"" },
        { SCE_AHK_NUMBER, 63130, L"Number", L"fore:#2F4F7F", L"" },
        { SCE_AHK_IDENTIFIER, 63129, L"Identifier", L"fore:#CF2F0F", L"" },
        { SCE_AHK_VARREF, 63355, L"Variable dereferencing", L"fore:#CF2F0F; back:#E4FFE4", L"" },
        { SCE_AHK_LABEL, 63235, L"Label", L"fore:#000000; back:#FFFFA1", L"" },
        { SCE_AHK_WORD_CF, 63356, L"Flow of control", L"fore:#480048; bold", L"" },
        { SCE_AHK_WORD_CMD, 63236, L"Command", L"fore:#004080", L"" },
        { SCE_AHK_WORD_FN, 63247, L"Function name", L"fore:#0F707F; italics", L"" },
        { SCE_AHK_WORD_DIR, 63357, L"Directive", L"fore:#F04020; italics", L"" },
        { SCE_AHK_WORD_KB, 63358, L"Keys & buttons", L"fore:#FF00FF; bold", L"" },
        { SCE_AHK_WORD_VAR, 63359, L"Built-in variables", L"fore:#CF00CF; italics", L"" },
        { SCE_AHK_WORD_SP, 63295, L"Special", L"fore:#0000FF; italics", L"" },
        { SCE_AHK_WORD_UD, 63300, L"User defined", L"fore:#800020", L"" },
        { SCE_AHK_VARREFKW, 63360, L"Variable keyword", L"fore:#CF00CF; italics; back:#F9F9FF", L"" },
        { SCE_AHK_ERROR, 63325, L"Error", L"back:#FFC0C0", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};

// [2e]: Lua LPeg Lexers #251
#ifdef LPEG_LEXER
EDITLEXER lexLPEG = { SCLEX_LPEG, 63037, L"LPEG", L"", L"", &KeyWords_NULL, NULL_COMMENT_INFO, {
        { 0, 63126, L"Default", L"", L"" },
        { -1, 00000, L"", L"", L"" }
      }
};
#endif
// [/2e]


PEDITLEXER pLexArray[NUMLEXERS] = {
    &lexDefault,
    &lexCONF,
    &lexASN1,
    &lexASM,
    &lexAHK,
    &lexAWK,
    &lexBASH,
    &lexBAT,
    &lexCS,
    &lexCPP,
    &lexCOFFEESCRIPT,
    &lexINI,
    &lexCSS,
    &lexD,
    &lexDIFF,
    &lexJAVA,
    &lexJS,
    &lexLisp,
    &lexLua,
    &lexMAK,
    &lexMarkdown,
    &lexNSIS,
    &lexCAML,
    &lexPAS,
    &lexPL,
    &lexPS,
    &lexPY,
    &lexRC,
    &lexRUBY,
    &lexRust,
    &lexSQL,
    &lexTeX,
    &lexVBS,
    &lexVB,
    &lexHTML,
    &lexXML,
    &lexYaml
#ifdef LPEG_LEXER
    , &lexLPEG
#endif
};

PEDITLEXER pLexCurrent = &lexDefault;
