/*
  Comment lexer modules from "scintilla\src\Catalogue.cxx" not used by Notepad2
  (c) Florian Balmer 2011
*/

  var lex = new Array(
    "lmAsm",
    "lmBatch",
    "lmConf",
    "lmCPP",
    "lmCss",
    "lmDiff",
    "lmHTML",
    "lmMake",
    "lmNull",
    "lmPascal",
    "lmPerl",
    "lmPowerShell",
    "lmProps",
    "lmPython",
    "lmSQL",
    "lmVB",
    "lmVBScript",
    "lmXML",
    "lmAsn1", /* [n2e] */
    "lmBash", /* [n2e] */
    "lmCaml", /* [n2e] */
    "lmCoffeeScript", /* [n2e] */
    "lmD", /* [n2e] */
    "lmLISP", /* [n2e] */
    "lmTeX" /* [n2e] */
  );

  var fso = new ActiveXObject("Scripting.FileSystemObject");
  var fh = fso.OpenTextFile("scintilla\\src\\Catalogue.cxx",1,0);
  if (!fh.AtEndOfStream) {
    var str = fh.ReadAll();
    str = str.replace(
      /^(\s*)\/\/(LINK_LEXER)/gim,
      "$1$2");
    var re = new RegExp("^(\\s*)(LINK_LEXER\\((?!"+lex.join("|")+")\\w+\\);)","gim")
    str = str.replace(re,"$1//$2");
    fh.Close();
    var fh = fso.OpenTextFile("scintilla\\src\\Catalogue.cxx",2,0);
    fh.Write(str);
  }
