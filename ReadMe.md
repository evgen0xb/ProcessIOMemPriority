
## About
**ProcessIOMemPriority** is command line utility for run, query and set CPU, In/Out and Memory priority of Windows processes.

The program is provided as-is and without any warranty under the GPLv3 license.
https://github.com/evgen0xb/ProcessIOMemPriority

Based on Process Priority utility by Charles Nevill and Process Hacker by Wen Jia Liu.
https://web.archive.org/web/20110130041716/http://blog.misterfoo.com/2010/07/process-priority-utility.html
https://sourceforge.net/projects/processhacker/files/

## Usage
`ProcessIOMemPriority.exe <mode> <options...> [-fail fvalue]`

All parameters are case-insensitive.

If an error occurs, the optional \<fvalue\> will be returned as %ERRORLEVEL%. The default \<fvalue\> is -1.

To set/change priority you must run ProcessIOMemPriority as Administrator or SYSTEM. Otherwise, error c0000061 (STATUS_PRIVILEGE_NOT_HELD) will occur, meaning that the client does not have the required privileges. In Windows XP, you can only change the CPU priority; setting a different priority type cause
an error 0xC0000003 (STATUS_INVALID_INFO_CLASS), meaning it is not supported.

* Query mode:
`ProcessIOMemPriority.exe query -pid pid`

Retrieves the priority info for process \<pid\>.

    Return %ERRORLEVEL%:
    0:         success
    fvalue:    error performing request

example: `ProcessIOMemPriority.exe query -pid 1234`

Query current cpu, in/out and memory priorities of process with decimal PID value 1234.

* Set mode:
`ProcessIOMemPriority.exe set -pid pid { [-preset pres] [-cpupriority cpu] [-inoutpriority io] [-mempriority mem] }`

Sets the priority of process \<pid\>.
See 'Presets and priorities' section for supported values.

    Return %ERRORLEVEL%:
    0:         success
    fvalue:    error change priority

example: `ProcessIOMemPriority.exe set -pid 1234 -preset low -inoutpriority high`

Set cpu and memory priorities as \<low\> by preset, but in/out priority as \<high\> for the process with decimal PID value 1234.
> [!NOTE] 
> If You want to mix preset and custom priorities, You must use option -preset first before any custom priorities (-cpupriority, -inoutpriority and -mempriority)!

* Start mode:
`ProcessIOMemPriority.exe start { [-preset pres] [-cpupriority cpu] [-inoutpriority io] [-mempriority mem] } [-nowait] [-wd workingdirectory] -cmd <command> [args]`

Run command line \<command\> [args] with priority.
See 'Presets and priorities' section for supported values.

    -nowait    - do not wait for the running child process to complete.
    -wd        - set the working directory for the child process,
if the \<-wd\> option is not specified, the working directory of the current process is inherited.

    Return %ERRORLEVEL% if option -nowait used:
    PID:       child process ID
    fvalue:    error creating process or change priority
    Return %ERRORLEVEL% if option -nowait NOT used:
    exitcode:  child process exit code
    fvalue:    error creating process or change priority

example: `ProcessIOMemPriority.exe start -inoutpriority high -wd c:\windows -cmd wstunnel.exe -v --udp --udpTimeoutSec -1 -L 8043:127.0.0.1:51820 wss://wss.domain.com --upgradePathPrefix siteprefix`

Runs wstunnel.exe with some parameters with in/out priority \<high\> but don't change other priorities and wait until the wstunnel is stopped.

* Presets and priorities:

Windows application has only 2 memory priority values: `Low` and `Normal`;

3 in/out priority values: `Low`, `Normal` and `High`.

and 6 CPU priority values: `Idle`, `BelowNormal`, `Normal`, `AboveNormal`, `High` and `Realtime`.

Windows creates a new user process with all priorities set to `Normal` by default.

    Preset      CPU             In/Out     Memory
    low    ->   Below Normal    Low        Low
    normal ->   Normal          Normal     Normal
    high   ->   High            High       Normal

## Build.

Use the vs2015.sln file to edit the project's source code in modern versions of Visual Studio.

Use the vs2005.sln file to compile the project into compact code with Windows XP 32/64 support using Visual Studio 2005 (for x64 compilation, you will need the Pro version).

You can additionally compress the executable file using UPX (https://github.com/upx/upx)
or Upack v0.399 - Ultimate PE Packer (https://web.archive.org/web/20060824085857/http://dwing.51.net/)
    
    upx.exe --best --ultra-brute --all-methods --all-filters --force -v "ProcessIOMemPriority.exe"

## Batch example.

    @ECHO OFF
    ECHO.Using ProcessIOMemPriority.exe in batch example.
    
    SET "mypath=%~p0"
    SET "mydisk=%~d0"
    SET "mypath=%mypath:~0,-1%"
    CD /d "%mydisk%%mypath%"
    
    :: ProcessIOMemPriority.exe start -nowait -cpupriority idle -cmd notepad.exe
    :: output:
    :: ...
    :: ProcessIOMemPriority version 2.1
    :: RUN [notepad.exe]
    :: Create Process with PID [3080] and CPU priority 'Idle'
    
    SET PRESET=low
    SET TESTAPP=notepad.exe
    SET "CHILDPID="
    
    REM asynchronous start (with -nowait option), get PID or Error code for child process
    FOR /F "usebackq tokens=2 delims=[]" %%A IN (`ProcessIOMemPriority.exe start -nowait -preset %PRESET% -cmd %TESTAPP%`) DO SET "CHILDPID=%%A"
    
    IF "%CHILDPID%" == "" GOTO :fail
    REM Error code starts with "0x"
    SET TESTFORERROR=%CHILDPID:~0,2%
    IF "%TESTFORERROR%" == "0x" GOTO :fail
    
    ECHO.
    ECHO.Created process [%TESTAPP%] with preset '%PRESET%', PID=%CHILDPID%
    
    REM Query priority info
    ProcessIOMemPriority.exe query -pid %CHILDPID%
    
    ECHO.
    ECHO.Change CPU priority to abovenormal:
    ProcessIOMemPriority.exe set -pid %CHILDPID% -cpupriority abovenormal
    
    REM Query priority info
    ProcessIOMemPriority.exe query -pid %CHILDPID%
    
    ECHO.
    ECHO.Change Memory priority to Normal:
    ProcessIOMemPriority.exe set -pid %CHILDPID% -mempriority normal
    
    REM Query priority info
    ProcessIOMemPriority.exe query -pid %CHILDPID%
    
    ECHO.
    ECHO.Change In/Out priority to High:
    ProcessIOMemPriority.exe set -pid %CHILDPID% -inoutpriority high
    
    REM Query priority info
    ProcessIOMemPriority.exe query -pid %CHILDPID%
    
    ECHO.
    ECHO.Change In/Out priority to Normal and CPU to Idle:
    ProcessIOMemPriority.exe set -pid %CHILDPID% -inoutpriority normal -cpupriority idle
    
    REM Query priority info
    ProcessIOMemPriority.exe query -pid %CHILDPID%
    
    ECHO.
    ECHO.Done.
    PAUSE
    
    REM Pause 1 second example:
    CHOICE /T 1 /D y > NUL
    taskkill.exe /F /PID %CHILDPID%
    GOTO :EOF
    
    :fail
    ECHO.
    ECHO.FAIL, ERROR CODE: %CHILDPID%
    PAUSE
    
    GOTO :EOF
