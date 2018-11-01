# Building Notepad2e with ICU support (International Components for Unicode, http://site.icu-project.org/)

## I. Set up compiler

### Requirements
1. Visual C++ 14.1 (Visual Studio 2017)
2. Cygwin with the following installed:
* bash
* GNU make
* ar
* ranlib

### Setup
1. Install Visual Studio 2017 (15.7.6+).
2. Install Cygwin, install next packages: bash, GNU make, ar, ranlib 

### Command prompt
Open the *x86 VS2017 xXX Native Tools Command Prompt*.

## II. Download prerequisites/sources
1. Get boost sources: `boost_1_68_0.zip` (http://www.boost.org/users/download/)
2. Get ICU sources, commit `55ecf77306e2f73ddced8d8e6deb510dfbffd94e`: [icu-55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip](https://github.com/unicode-org/icu/archive/55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip) (https://github.com/unicode-org/icu)

## III. Set up the build directory
1. Unzip `boost_1_68_0.zip`, e.g. to `C:\\Program Files\\boost`
2. Setup BOOST_ROOT environment variable to the boost destination path, e.g. `C:\\Program Files\\boost\\boost_1_68_0`
3. Unzip `icu-55ecf77306e2f73ddced8d8e6deb510dfbffd94e.zip`, e.g. to `C:\\icu`
4. Setup ICU_ROOT environment variable to the ICU destination path, e.g. `C:\\icu\\dist` (`dist` is output subfolder for ICU build)

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
```
LINK.c=		LINK.EXE -subsystem:console,5.01 $(LDFLAGS)
LINK.cc=	LINK.EXE -subsystem:console,5.01 $(LDFLAGS)
```

## V. Build ICU library
1. Open *x86 VS2017 xXX Native Tools Command Prompt* and switch to the ICU directory:
```
> cd C:\icu\icu4c\source
```
2. Run command:
```
C:\icu\icu4c\source> bash runConfigureICU Cygwin/MSVC -prefix=/cygdrive/c/icu/dist --enable-static --disable-shared --enable-release --disable-debug
```
3. Run command:
```
C:\icu\icu4c\source> make && make install
```
Output folder for compiled static libraries and required include headers is ICU_ROOT folder (`C:\\icu\\dist`)

## VI. Patch boost source code
1. Patch file `boost_1_68_0\libs\regex\build\jamfile.v2`:
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
1. Open *x86 VS2017 xXX Native Tools Command Prompt* and switch to the boost directory:
```
> cd %BOOST_ROOT%
```
2. Run command:
```
C:\Program Files\boost\boost_1_68_0> bootstrap.bat
```
3. Run command:
```
C:\Program Files\boost\boost_1_68_0> b2 address-model=32 link=static runtime-link=static --with-regex --stagedir=stage/86 define=_USING_V110_SDK71_=1 define=U_STATIC_IMPLEMENTATION=1 -sHAVE_ICU=1 -sICU_PATH="C:\icu\dist" -sICU_LINK="/LIBPATH:C:\icu\dist\lib sicuucd.lib sicudtd.lib sicuind.lib sicuiod.lib sicutud.lib advapi32.lib" --disable-debug --enable-release
```
Output folder for compiled static regex libary is `C:\Program Files\boost\boost_1_68_0\stage\86\lib`.

## VIII. Create ICU regex static library
1. Copy boost regex lib `libboost_regex-vc141-mt-s-x32-1_68.lib` to `C:\Program Files\boost\boost_1_68_0\stage\lib`
2. Copy ICU libs from `C:\\icu\\dist\\lib` to `C:\Program Files\boost\boost_1_68_0\stage\lib`
3. Open *x86 VS2017 xXX Native Tools Command Prompt* and navigate to `C:\Program Files\boost\boost_1_68_0\stage\lib`
4. Merge required libs into single `icuregex.lib` file:
C:\Program Files\boost\boost_1_68_0\stage\lib> lib /OUT:icuregex86.lib libboost_regex-vc141-mt-s-x32-1_68.lib sicudt.lib sicuin.lib sicuuc.lib

## IX. Compile Notepad2e
1. Open Notepad2e project with VS2017
2. Switch to `Release ICU` - `x86` configuration
3. Build solution
