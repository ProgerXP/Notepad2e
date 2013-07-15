@echo off
echo #ifndef _H_TIME_>%1
echo #define _H_TIME_>>%1
echo #define H_TIMESTAMP L"%DATE% %TIME%" >>%1
echo #endif>>%1