REM Creative Commons Attribution 4.0 International Public License
REM 2026 (L) evgen_b

SET "mypath=%~p0"
SET "mydisk=%~d0"
SET "mypath=%mypath:~0,-1%"
CD /d "%mydisk%%mypath%"

:: VS2005

RD /s /q debug

RD /s /q vs2005\Debug
RD /s /q vs2005\Release
RD /s /q vs2005\x64

DEL /f /q vs2005\*.user

RD /s /q x64\debug

DEL /f /q vs2005.ncb
ATTRIB.EXE -r -a -s -h vs2005.suo
DEL /f /q vs2005.suo

:: +++ VS2015

ATTRIB.EXE -r -a -s -h .vs
RD /s /q .vs
DEL /f /q vs2015.VC.db
DEL /f /q vs2015\*.user

DEL /f /q release\*.iobj
DEL /f /q release\*.ipdb
DEL /f /q release\*.pdb
RD  /s /q debug

DEL /f /q x64\release\*.iobj
DEL /f /q x64\release\*.ipdb
DEL /f /q x64\release\*.pdb
RD  /s /q x64\debug

RD /s /q vs2015\Debug
RD /s /q vs2015\Release
RD /s /q vs2015\x64

DEL /f /q SRC\*.aps

PAUSE
