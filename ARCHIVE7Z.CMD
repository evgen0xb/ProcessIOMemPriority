REM Creative Commons Attribution 4.0 International Public License
REM 2026 (L) evgen_b

SET "mypath=%~p0"
SET "mydisk=%~d0"
SET "mypath=%mypath:~0,-1%"
CD /d "%mydisk%%mypath%"

REM make 7z archive for release

SET "archiver=c:\Program Files (x86)\7-Zip\7z.exe"
SET arcname=ProcessIOMemPriority
SET reldir=OUT

DEL /f /q "%arcname%.7z"
RD /s /q %reldir%
MD %reldir%\x86
MD %reldir%\AMD64

COPY Release\ProcessIOMemPriority.exe %reldir%\x86
COPY x64\Release\ProcessIOMemPriority.exe %reldir%\AMD64
COPY ReadMe.md %reldir%\

"%archiver%" a -t7z -ssw -slt -mx=9 -myx=9 -ms=256m "%arcname%" -r ".\%reldir%\*.*"

RD /s /q %reldir%

PAUSE
