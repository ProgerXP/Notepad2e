1. Download boost sources from here: http://www.boost.org/users/download/
1. Unzip boost_1_68_0.zip, e.g. to C:\\Program Files\\boost
2. Setup BOOST_ROOT environment variable to the destination path, e.g. C:\\Program Files\\boost\\boost_1_68_0
3. Open VS2017 native tools command prompt (x86/x64), run command: "cd %BOOST_ROOT%"
4. Run command: "bootstrap.bat"
5. Run command:  
for x86:  
"b2 runtime-link=static link=static --with-regex --stagedir=stage/x86"  
for x64:  
"b2 address-model=64 architecture=x86 runtime-link=static link=static --with-regex --stagedir=stage/x64"  
6. Copy next files created in %BOOST_ROOT%\\stage\\(x86|x64)\\lib to %BOOST_ROOT%\\stage\\lib:  
for x86:  
libboost_regex-vc141-mt-s-x32-1_68.lib  
libboost_regex-vc141-mt-sgd-x32-1_68.lib  
for x64:  
libboost_regex-vc141-mt-s-x64-1_68.lib  
libboost_regex-vc141-mt-sgd-x64-1_68.lib   
