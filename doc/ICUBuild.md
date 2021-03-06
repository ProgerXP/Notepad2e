# Building Notepad2e with ICU support (International Components for Unicode, http://site.icu-project.org/)

In this text, xXX means x86 or x64 depending on the required build version.

## I. Set up compiler

### Requirements
1. Visual C++ 14.1 (Visual Studio 2017)
2. Cygwin with the following installed:
* bash
* GNU make

### Setup
1. Install Visual Studio 2017 (15.7.6+).
2. Install Cygwin, install next packages: bash, GNU make

### Command prompt
Open the *xXX Native Tools Command Prompt for VS 2017*.

## II. Download prerequisites/sources
1. Get boost sources: `boost_1_68_0.zip` (http://www.boost.org/users/download/)
2. Get ICU sources, commit `55ecf77306e2f73ddced8d8e6deb510dfbffd94e`: [icu-55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip](https://github.com/unicode-org/icu/archive/55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip) (https://github.com/unicode-org/icu)

## III. Set up the build directory
1. Unzip `boost_1_68_0.zip`, e.g. to `C:\Program Files\boost`
2. Rename `C:\Program Files\boost\boost_1_68_0` to `C:\Program Files\boost\boost_1_68_0_icu`
3. Setup BOOST_ROOT_ICU environment variable to the boost destination path, e.g. `C:\Program Files\boost\boost_1_68_0_icu`
4. Unzip `icu-55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip`, e.g. to `C:\icu`
5. Setup ICU_ROOT environment variable to the ICU destination path, e.g. `C:\icu\dist` (`dist` is output subfolder for ICU build)

## IV. Patch ICU source code
Now ICU sources need to be patched to support Windows XP.
1. Patch ICU file `icu4c\source\common\putil.cpp`:
```
#if U_PLATFORM_HAS_WINUWP_API == 0
    // If not a Universal Windows App, we'll need user default language.
    // Vista and above should use Locale Names instead of LCIDs
    int length = GetUserDefaultLocaleName(windowsLocale, UPRV_LENGTHOF(windowsLocale));
#else
```
change to:
```
#if U_PLATFORM_HAS_WINUWP_API == 0
    int length = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SNAME, windowsLocale, UPRV_LENGTHOF(windowsLocale));
#else
```
2. Patch `icu4c\source\runConfigureICU` file:
```
        THE_OS="Windows with Cygwin"
        THE_COMP="Microsoft Visual C++"
        CC=cl; export CC
        CXX=cl; export CXX
        RELEASE_CFLAGS='-Gy -MD'
        RELEASE_CXXFLAGS='-Gy -MD'
        DEBUG_CFLAGS='-Zi -MDd'
        DEBUG_CXXFLAGS='-Zi -MDd'
        DEBUG_LDFLAGS='-DEBUG'
```
change to:
```
        THE_OS="Windows with Cygwin"
        THE_COMP="Microsoft Visual C++"
        CC=cl; export CC
        CXX=cl; export CXX
        RELEASE_CFLAGS='-Gy -MT /D_USING_V110_SDK71_'
        RELEASE_CXXFLAGS='-Gy -MT /D_USING_V110_SDK71_'
        DEBUG_CFLAGS='-Zi -MTd /D_USING_V110_SDK71_'
        DEBUG_CXXFLAGS='-Zi -MTd /D_USING_V110_SDK71_'
        DEBUG_LDFLAGS='-DEBUG'
```
3. Patch `icu4c\source\config\mh-cygwin-msvc`:
```
LINK.c=		LINK.EXE -subsystem:console $(LDFLAGS)
LINK.cc=	LINK.EXE -subsystem:console $(LDFLAGS)
```
change to:
* For **x86**:
```
LINK.c=		LINK.EXE -subsystem:console,5.01 $(LDFLAGS)
LINK.cc=	LINK.EXE -subsystem:console,5.01 $(LDFLAGS)
```
* For **x64**:
```
LINK.c=		LINK.EXE -subsystem:console,5.02 $(LDFLAGS)
LINK.cc=	LINK.EXE -subsystem:console,5.02 $(LDFLAGS)
```

## V. Build ICU library
1. Open *xXX Native Tools Command Prompt for VS 2017* and switch to the ICU directory:
```
> cd %ICU_ROOT%\..\icu4c\source
```
2. Run command:
```
C:\icu\icu4c\source> bash -exec "ICU_ROOT_UNIX=$(cygpath -u ${ICU_ROOT});bash runConfigureICU Cygwin/MSVC -prefix=${ICU_ROOT_UNIX} --enable-static --disable-shared --enable-release --disable-debug"
```
3. Run command:
```
C:\icu\icu4c\source> make && make install
```
Output folder for compiled static libraries and required include headers is ICU_ROOT folder (`C:\icu\dist`)

## VI. Patch boost source code
1. Patch file `boost_1_68_0_icu\libs\regex\build\jamfile.v2`:
```
   if $(ICU_LINK)
   {
      ICU_OPTS = <include>$(ICU_PATH)/include <linkflags>$(ICU_LINK) <dll-path>$(ICU_PATH)/bin <define>BOOST_HAS_ICU=1 <runtime-link>shared ;
   }
```
change to:
```
   if $(ICU_LINK)
   {
      ICU_OPTS = <include>$(ICU_PATH)/include <linkflags>$(ICU_LINK) <dll-path>$(ICU_PATH)/bin <define>BOOST_HAS_ICU=1 <runtime-link>static ;
   }
```

## VII. Build boost regex
1. Open *xXX Native Tools Command Prompt for VS 2017* and switch to the boost directory:
```
> cd %BOOST_ROOT_ICU%
```
2. Run command:
```
C:\Program Files\boost\boost_1_68_0_icu> bootstrap.bat
```
3. Run command:
* For **x86**:
```
C:\Program Files\boost\boost_1_68_0_icu> b2 address-model=32 link=static runtime-link=static --with-regex --stagedir=stage/x86 define=_USING_V110_SDK71_=1 define=U_STATIC_IMPLEMENTATION=1 -sHAVE_ICU=1 -sICU_PATH=%ICU_ROOT% -sICU_LINK="/LIBPATH:%ICU_ROOT%\lib sicuucd.lib sicudtd.lib sicuind.lib sicuiod.lib sicutud.lib advapi32.lib" --disable-debug --enable-release
```
* For **x64**:
```
C:\Program Files\boost\boost_1_68_0_icu> b2 address-model=64 architecture=x86 link=static runtime-link=static --with-regex --stagedir=stage/x64 define=_USING_V110_SDK71_=1 define=U_STATIC_IMPLEMENTATION=1 -sHAVE_ICU=1 -sICU_PATH=%ICU_ROOT% -sICU_LINK="/LIBPATH:%ICU_ROOT%\lib sicuucd.lib sicudtd.lib sicuind.lib sicuiod.lib sicutud.lib advapi32.lib" --disable-debug --enable-release
```
Output folder for compiled static boost regex libary is `%BOOST_ROOT_ICU%\stage\xXX\lib`.

## VIII. Create ICU regex static library
1. Copy ICU libs from `%ICU_ROOT%\lib` to `%BOOST_ROOT_ICU%\stage\xXX\lib`
2. Open *xXX Native Tools Command Prompt for VS 2017* and navigate to `%BOOST_ROOT_ICU%\stage\xXX\lib`
3. Merge required libs into single library:
* For **x86**:
```
C:\Program Files\boost\boost_1_68_0_icu\stage\x86\lib> lib /OUT:icuregex86.lib libboost_regex-vc141-mt-s-x32-1_68.lib sicudt.lib sicuin.lib sicuuc.lib
```
* For **x64**:
```
C:\Program Files\boost\boost_1_68_0_icu\stage\x64\lib> lib /OUT:icuregex64.lib libboost_regex-vc141-mt-s-x64-1_68.lib sicudt.lib sicuin.lib sicuuc.lib
```
4. Copy boost regex lib `libboost_regex-vc141-mt-s-xXX-1_68.lib` and ICU lib `icuregexXX.lib` to `%BOOST_ROOT_ICU%\stage\lib`

## IX. Compile Notepad2e
1. Open Notepad2e project with VS2017
2. Switch to `Release ICU` - `xXX` configuration
3. Build solution
