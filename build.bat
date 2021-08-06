@echo off

cl -W4 -wd4996 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -Zi -nologo main.c /DTESTING
