0. Download boost sources from here: http://www.boost.org/users/download/
1. Unzip boost_1_68_0.zip, e.g. to C:\\Program Files\\boost
2. Setup BOOST_ROOT environment variable to the destination path, e.g. C:\\Program Files\\boost\\boost_1_68_0
3. Open console window (cmd), run command: "cd %BOOST_ROOT%"
4. Run command: "bootstrap.bat"
5. Run command: "b2 --with-regex runtime-link=static link=static"
6. Check next files created in %BOOST_ROOT%\\stage\\lib:
	libboost_regex-vc141-mt-s-1_68.lib
	libboost_regex-vc141-mt-sgd-1_68.lib
