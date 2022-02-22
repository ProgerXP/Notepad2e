@echo off
set DEST_FILE=%1_version.h

echo #ifndef _H_VERSION_> %DEST_FILE%
echo #define _H_VERSION_>>%DEST_FILE%
echo #define H_TIMESTAMP    L"%DATE% %TIME%">>%DEST_FILE%
echo #define BUILD_YEAR_STR "%DATE:~-4%">>%DEST_FILE%
echo #endif>>%DEST_FILE%