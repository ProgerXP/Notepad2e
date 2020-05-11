### How to test LPEG custom scheme:

1. Create folder structure in Notepad2e folder:
> **`lexlua`**
>> **`themes`**
2. Copy `lexer.lua` from `scintilla\lua\lexlua` to **`lexlua`**
1. Copy original lexer (i.e `cuda.lua`) from `scintilla\lua\lexlua` to **`lexlua`**, rename to `custom.lua`
1. Copy `default.lua` from `scintilla\lua\lexlua\themes` to **`themes`**
1. Run Notepad2e, select `LPEG Lexer` (last one in the list): View> Syntax Scheme
