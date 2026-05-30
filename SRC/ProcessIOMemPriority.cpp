/*
The program is provided as-is and without any warranty under the GPLv3 license.
https://github.com/evgen0xb/ProcessIOMemPriority

Additional permission under GNU GPL version 3 section 7 (https://github.com/microsoft/WindowsAppSDK/discussions/3511#discussioncomment-10121323):
If you modify this Program, or any covered work, by linking or combining it with the Microsoft Visual C++ Redistributable, Windows SDK (or a
modified version of these libraries), containing parts covered by the terms of the Microsoft Software License, the licensors of this Program
grant you additional permission to convey the resulting work.

static build
c/c++ -> создание кода -> библиотека времени выполнения -> многопоточная (/MT)
DLL build
c/c++ -> создание кода -> библиотека времени выполнения -> многопоточный DLL (/MD)

Use the vs2015.sln file to edit the project's source code in modern versions of Visual Studio.
Use the vs2005.sln file to compile the project into compact code with Windows XP 32/64 support using
Visual Studio 2005 (for x64 compilation, you will need the Pro version).
*/

// Microsoft Visual Studio 2005

// #undef UNICODE
// #undef _UNICODE

#pragma warning					(disable : 4996)
#define _WIN32_WINNT			0x0501 // XP and above
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <Shlwapi.h>
#pragma comment					(lib, "shlwapi.lib")
#include "compiledate.h"



#define MAXERRORPRINTBUF		512
LPCTSTR VERSION					= TEXT("2.1");
const TCHAR const COMPILEDATE[]	= __DATE_yyyy_mm_dd__;



typedef							long NTSTATUS; // vs2005 fix
// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55
#define STATUS_SUCCESS			0 // vs2005 fix



// ****************************************************************************************************************************
// ******************************************************** Privileges ********************************************************
// ****************************************************************************************************************************

// /winsdk-10/blob/master/Include/10.0.10240.0/shared/sal.h #define _Out_writes_bytes_to_opt_(size,count)  _SAL2_Source_(_Out_writes_bytes_to_opt_, (size,count), _Pre_opt_bytecap_(size) _Post_valid_impl_ _Post_bytecount_(count))
// /include/10.0.10240.0/shared/no_sal2.h
#ifdef _Out_writes_bytes_to_opt_
#undef _Out_writes_bytes_to_opt_
#endif
#define _Out_writes_bytes_to_opt_(s,c)

// for PhpEnablePrivileges():

// systeminformer\phnt\include\ntseapi.h; https://gitlab.winehq.org/wine/wine/blob/master/include/winternl.h
#define SE_MACHINE_ACCOUNT_PRIVILEGE			(6L)
#define SE_TCB_PRIVILEGE						(7L)
#define SE_SECURITY_PRIVILEGE					(8L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE				(9L)
#define SE_LOAD_DRIVER_PRIVILEGE				(10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE				(11L)
#define SE_SYSTEMTIME_PRIVILEGE					(12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE		(13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE			(14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE			(15L)
#define SE_CREATE_PERMANENT_PRIVILEGE			(16L)
#define SE_BACKUP_PRIVILEGE						(17L)
#define SE_RESTORE_PRIVILEGE					(18L)
#define SE_SHUTDOWN_PRIVILEGE					(19L)
#define SE_DEBUG_PRIVILEGE						(20L)
#define SE_AUDIT_PRIVILEGE						(21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE			(22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE				(23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE			(24L)
#define SE_UNDOCK_PRIVILEGE						(25L)
#define SE_SYNC_AGENT_PRIVILEGE					(26L)
#define SE_ENABLE_DELEGATION_PRIVILEGE			(27L)
#define SE_MANAGE_VOLUME_PRIVILEGE				(28L)
#define SE_IMPERSONATE_PRIVILEGE				(29L)
#define SE_CREATE_GLOBAL_PRIVILEGE				(30L)
#define SE_TRUSTED_CREDMAN_ACCESS_PRIVILEGE		(31L)
#define SE_RELABEL_PRIVILEGE					(32L)
#define SE_INC_WORKING_SET_PRIVILEGE			(33L)
#define SE_TIME_ZONE_PRIVILEGE					(34L)
#define SE_CREATE_SYMBOLIC_LINK_PRIVILEGE		(35L)

// systeminformer\phnt\include\ntpsapi.h
#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)



// The NtOpenProcessToken routine opens the access token associated with a process, and returns a handle that can be used to access that token.
// @param ProcessHandle Handle to the process whose access token is to be opened. The handle must have PROCESS_QUERY_INFORMATION access.
// @param DesiredAccess ACCESS_MASK structure specifying the requested types of access to the access token.
// @param TokenHandle Pointer to a caller-allocated variable that receives a handle to the newly opened access token.
// @return NTSTATUS Successful or errant status.
// @sa https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntopenprocesstoken

typedef NTSTATUS (NTAPI *NtOpenProcessTokenFn)(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, PHANDLE TokenHandle);
static NtOpenProcessTokenFn NtOpenProcessToken; // systeminformer\phnt\include\ntseapi.h



// The NtAdjustPrivilegesToken routine enables or disables privileges in the specified access token.
// @param TokenHandle Handle to the token that contains the privileges to be modified. The handle must have TOKEN_ADJUST_PRIVILEGES access.
// @param DisableAllPrivileges Specifies whether the function disables all of the token's privileges. If this value is TRUE, the function disables all privileges and ignores the NewState parameter.
// If it is FALSE, the function modifies privileges based on the information pointed to by the NewState parameter.
// @param NewState A pointer to a TOKEN_PRIVILEGES structure that specifies an array of privileges and their attributes. If DisableAllPrivileges is TRUE, the function ignores this parameter.
// @param BufferLength Specifies the size, in bytes, of the buffer pointed to by the PreviousState parameter. This parameter can be zero if the PreviousState parameter is NULL.
// @param PreviousState A pointer to a buffer that the function fills with a TOKEN_PRIVILEGES structure that contains the previous state of any privileges that the function modifies.
// @param ReturnLength A pointer to a variable that receives the required size, in bytes, of the buffer pointed to by the PreviousState parameter. This parameter can be NULL if PreviousState is NULL.
// @return NTSTATUS Successful or errant status.
// @sa https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges

typedef NTSTATUS (NTAPI *NtAdjustPrivilegesTokenFn)(HANDLE TokenHandle, BOOLEAN DisableAllPrivileges, PTOKEN_PRIVILEGES NewState, ULONG BufferLength, _Out_writes_bytes_to_opt_(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState, PULONG ReturnLength);
static NtAdjustPrivilegesTokenFn NtAdjustPrivilegesToken; // systeminformer\phnt\include\ntseapi.h
// NtAdjustPrivilegesToken(tokenHandle, FALSE, privileges, 0, NULL, NULL);



// The NtClose routine closes the specified handle.
// @param Handle The handle being closed.
// @return NTSTATUS Successful or errant status.
// @sa https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-zwclose

typedef NTSTATUS (NTAPI *NtCloseFn)(HANDLE Handle);
static NtCloseFn NtClose; // systeminformer-master\phnt\include\ntobapi.h



// **************************************************************************************************************************
// ******************************************************** Priority ********************************************************
// **************************************************************************************************************************

namespace Priority {
	enum Type
	{
		// these values determined by poking around in the debugger - use at your own risk!
		ProcessInformationMemoryPriority = 0x27,
		ProcessInformationInOutPriority = 0x21,

		// Windows Vista (msdn - priorityio.doc)
		// https://github.com/winsiderss/systeminformer/blob/master/phnt/include/ntioapi.h
		// The IO_PRIORITY_HINT enumeration type specifies the priority hint for an IRP.
		// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ne-wdm-_io_priority_hint
		IoPriorityVeryLow = 0,		// Defragging, content indexing and other background I/Os.
		IoPriorityLow,				// Prefetching for applications.
		IoPriorityNormal,			// Normal I/Os (Default).
		IoPriorityHigh,				// Used by filesystems for checkpoint I/O.
		IoPriorityCritical,			// Used by memory manager. Not available for applications.
		MaxIoPriorityTypes,			// Marks the limit for priority hints. Any priority hint value must be less than MaxIoPriorityTypes.

		// extra internal value for the application only:
		DontChangePriority = -1,
	};
} // VS2005 fix

// https://github.com/winsiderss/systeminformer/blob/master/phnt/include/ntmmapi.h
// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/ns-processthreadsapi-memory_priority_information
// Page/memory priorities - Windows 8?

#define MEMORY_PRIORITY_LOWEST           0
#define MEMORY_PRIORITY_VERY_LOW         1
#define MEMORY_PRIORITY_LOW              2
#define MEMORY_PRIORITY_MEDIUM           3
#define MEMORY_PRIORITY_BELOW_NORMAL     4
#define MEMORY_PRIORITY_NORMAL           5 // (Default)
#define MEMORY_PRIORITY_ABOVE_NORMAL     6 // rev
#define MEMORY_PRIORITY_HIGH             7 // rev

typedef NTSTATUS (NTAPI *NtQueryInformationProcessFn)( HANDLE process, ULONG infoClass, void* data, ULONG dataSize, ULONG* outSize );
static NtQueryInformationProcessFn NtQueryInformationProcess;

typedef NTSTATUS (NTAPI *NtSetInformationProcessFn)( HANDLE process, ULONG infoClass, const void* data, ULONG dataSize );
static NtSetInformationProcessFn NtSetInformationProcess;

// ************************************************************************************************************************
// ******************************************************** Parser ********************************************************
// ************************************************************************************************************************

namespace Action
{
	const DWORD Unknown		= -1;
	const DWORD PrintHelp	= 0;
	const DWORD Query		= 1;
	const DWORD Set			= 2;
	const DWORD Start		= 3;
}

// presets:
LPCTSTR ANORMAL				= TEXT("normal");
LPCTSTR ALOW				= TEXT("low");
LPCTSTR AHIGH				= TEXT("high");
// action:
LPCTSTR AQUERY				= TEXT("query");
LPCTSTR ASET				= TEXT("set");
LPCTSTR ASTART				= TEXT("start");
LPCTSTR AHELP				= TEXT("help");
//
LPCTSTR APARSERR			= TEXT("Parser error :");
LPCTSTR AUNKNOWN			= TEXT("Unknown");

class clsPriorityOptions
{
public:
	DWORD	CPU, Memory, InOut;
	DWORD	Action;
	DWORD	TargetPid;
	LPTSTR	CmdLine;
	LPCTSTR	WorkingDirectory;
	DWORD	FailSpec;
	bool	NoWait;

	clsPriorityOptions();
	LPCTSTR ParseArgs(int argc, LPCTSTR argv[]);
	void DebugPrint();
	~clsPriorityOptions();

private:
	LPTSTR	ErrStr;
	void _allfree();
};

clsPriorityOptions::clsPriorityOptions()
{
	CPU			= Priority::DontChangePriority;
	Memory		= Priority::DontChangePriority;
	InOut		= Priority::DontChangePriority;
	Action		= Action::Unknown;
	TargetPid	= NULL;
	CmdLine		= NULL;
	WorkingDirectory = NULL;
	NoWait		= false;
	ErrStr		= NULL;
	FailSpec	= -1;
}

void clsPriorityOptions::_allfree() { if (ErrStr) { free((void *)ErrStr); ErrStr = NULL; } }

clsPriorityOptions::~clsPriorityOptions() { _allfree(); }

LPCTSTR clsPriorityOptions::ParseArgs(int argc, LPCTSTR argv[])
{
	// ParseArgs == NULL	: Parse OK
	// ParseArgs != NULL	: ErrorStringPtr

	enum Type {
		NextIsOptionName = 0,
		NextIsValuePID,
		NextIsValuePresetPriority,
		NextIsValueCPUPriority,
		NextIsValueInOutPriority,
		NextIsValueMemPriority,
		NextIsValueWorkDir,
		NextIsValueFailSpec,
	};

	_allfree();

	// skip "ProcessIOMemPriority.exe"
	CmdLine = PathGetArgs(GetCommandLine());
	int tokentype = NextIsOptionName;

	for (int i = 1; i < argc; i++)
	{
		//_tprintf(TEXT("argv[%i]=%s\n"), i, argv[i]);
		if (tokentype == NextIsOptionName)
		{

			if (_tcsicmp(argv[i], AQUERY) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				Action = Action::Query;
			}
			else if (_tcsicmp(argv[i], ASET) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				Action = Action::Set;
			}
			else if (_tcsicmp(argv[i], ASTART) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				Action = Action::Start;
			}
			else if (_tcsicmp(argv[i], AHELP) == 0 || _tcsicmp(argv[i], TEXT("-help")) == 0 ||
				_tcsicmp(argv[i], TEXT("-h")) == 0 || _tcsicmp(argv[i], TEXT("/h")) == 0 || _tcsicmp(argv[i], TEXT("/?")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				Action = Action::PrintHelp;
				break;
			}
			else if (_tcsicmp(argv[i], TEXT("-pid")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValuePID;
			}
			else if (_tcsicmp(argv[i], TEXT("-preset")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValuePresetPriority;
			}
			else if (_tcsicmp(argv[i], TEXT("-cpupriority")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValueCPUPriority;
			}
			else if (_tcsicmp(argv[i], TEXT("-inoutpriority")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValueInOutPriority;
			}
			else if (_tcsicmp(argv[i], TEXT("-mempriority")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValueMemPriority;
			}
			else if (_tcsicmp(argv[i], TEXT("-wd")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValueWorkDir;
			}
			else if (_tcsicmp(argv[i], TEXT("-cmd")) == 0 || _tcsicmp(argv[i], TEXT("--")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				break;
			}
			else if (_tcsicmp(argv[i], TEXT("-nowait")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				NoWait = true;
			}
			else if (_tcsicmp(argv[i], TEXT("-fail")) == 0)
			{
				CmdLine = PathGetArgs(CmdLine);
				tokentype = NextIsValueFailSpec;
			}
			else
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown option [%s]."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
		}
		else if (tokentype == NextIsValuePID)
		{
			CmdLine = PathGetArgs(CmdLine);
			TargetPid = (DWORD)_wtoi(argv[i]);
			if (TargetPid == 0)
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s PID [%s] must be a valid decimal number."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValueFailSpec)
		{
			CmdLine = PathGetArgs(CmdLine);
			FailSpec = (DWORD)_wtoi(argv[i]);
			if (FailSpec == 0)
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s Fail redefinition value [%s] must be a valid decimal non-null number."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValuePresetPriority)
		{
			CmdLine = PathGetArgs(CmdLine);

			if (_tcsicmp(argv[i], AHIGH) == 0)
			{
				CPU = HIGH_PRIORITY_CLASS;
				Memory = MEMORY_PRIORITY_NORMAL;
				InOut = Priority::IoPriorityHigh;
			}
			else if (_tcsicmp(argv[i], ANORMAL) == 0)
			{
				CPU = NORMAL_PRIORITY_CLASS;
				Memory = MEMORY_PRIORITY_NORMAL;
				InOut = Priority::IoPriorityNormal;
			}
			else if (_tcsicmp(argv[i], ALOW) == 0)
			{
				CPU = BELOW_NORMAL_PRIORITY_CLASS;
				Memory = MEMORY_PRIORITY_LOW;
				InOut = Priority::IoPriorityLow;
			}
			else
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown preset [%s]."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValueCPUPriority)
		{
			CmdLine = PathGetArgs(CmdLine);

			if (_tcsicmp(argv[i], AHIGH) == 0)
			{
				CPU = HIGH_PRIORITY_CLASS;
			}
			else if (_tcsicmp(argv[i], ANORMAL) == 0)
			{
				CPU = NORMAL_PRIORITY_CLASS;
			}
			else if (_tcsicmp(argv[i], TEXT("abovenormal")) == 0)
			{
				CPU = ABOVE_NORMAL_PRIORITY_CLASS;
			}
			else if (_tcsicmp(argv[i], TEXT("belownormal")) == 0)
			{
				CPU = BELOW_NORMAL_PRIORITY_CLASS;
			}
			else if (_tcsicmp(argv[i], TEXT("realtime")) == 0)
			{
				CPU = REALTIME_PRIORITY_CLASS;
			}
			else if (_tcsicmp(argv[i], TEXT("idle")) == 0)
			{
				CPU = IDLE_PRIORITY_CLASS;
			}
			else
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown CPU priority [%s]."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValueInOutPriority)
		{
			CmdLine = PathGetArgs(CmdLine);

			if (_tcsicmp(argv[i], AHIGH) == 0)
			{
				InOut = Priority::IoPriorityHigh;
			}
			else if (_tcsicmp(argv[i], ANORMAL) == 0)
			{
				InOut = Priority::IoPriorityNormal;
			}
			else if (_tcsicmp(argv[i], ALOW) == 0)
			{
				InOut = Priority::IoPriorityLow;
			}
			else
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown In/Out priority [%s]."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValueMemPriority)
		{
			CmdLine = PathGetArgs(CmdLine);

			if (_tcsicmp(argv[i], ANORMAL) == 0)
			{
				Memory = MEMORY_PRIORITY_NORMAL;
			}
			else if (_tcsicmp(argv[i], ALOW) == 0)
			{
				Memory = MEMORY_PRIORITY_LOW;
			}
			else
			{
				ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
				_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown Memory priority [%s]."), APARSERR, argv[i]); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
				break;
			}
			tokentype = NextIsOptionName;
		}
		else if (tokentype == NextIsValueWorkDir)
		{
			CmdLine = PathGetArgs(CmdLine);
			WorkingDirectory = argv[i];
			tokentype = NextIsOptionName;
		}
		else 
		{
			ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
			_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s unknown internal token number [%u]."), APARSERR, tokentype); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
			break;
		}
	} // end for

	if (!ErrStr && tokentype != NextIsOptionName)
	{
		ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
		_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s last option value expected."), APARSERR); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
	}
	else if (!ErrStr && Action == Action::Start && !CmdLine)
	{
		ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
		_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s start command line expected."), APARSERR); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
	}
	else if (!ErrStr && Action == Action::Unknown)
	{
		ErrStr = (LPTSTR)malloc(MAXERRORPRINTBUF * sizeof(TCHAR));
		_sntprintf(ErrStr, MAXERRORPRINTBUF, TEXT("%s main option expected."), APARSERR); ErrStr[MAXERRORPRINTBUF - 1] = NULL;
	}

	return ErrStr;
}



void printheader()
{
	_tprintf(TEXT("\n\nProcessIOMemPriority version %s build %s.\n"), VERSION, COMPILEDATE);
}

void PrintHelp()
{
	_putts(
TEXT("Based on Process Priority utility by Charles Nevill and Process Hacker by Wen Jia Liu.")
TEXT("\nhttps://web.archive.org/web/20110130041716/http://blog.misterfoo.com/2010/07/process-priority-utility.html")
TEXT("\nhttps://sourceforge.net/projects/processhacker/files/")
TEXT("\n")
TEXT("\nProcessIOMemPriority is command line utility for run, query and set CPU, In/Out and Memory priority of Windows processes.")
TEXT("\n")
TEXT("\nUsage: ProcessIOMemPriority.exe <mode> <options...> [-fail fvalue]")
TEXT("\n")
TEXT("\n    All parameters are case-insensitive.")
TEXT("\n")
TEXT("\n    If an error occurs, the optional <fvalue> will be returned as %ERRORLEVEL%.")
TEXT("\n    The default <fvalue> is -1.")
TEXT("\n")
TEXT("\n    To set/change priority you must run ProcessIOMemPriority as Administrator or SYSTEM.")
TEXT("\n    Otherwise, error c0000061 (STATUS_PRIVILEGE_NOT_HELD) will occur,")
TEXT("\n    meaning that the client does not have the required privileges.")
TEXT("\n    In Windows XP, you can only change the CPU priority; setting a different priority type cause")
TEXT("\n    an error 0xC0000003 (STATUS_INVALID_INFO_CLASS), meaning it is not supported.")
TEXT("\n")
TEXT("\n  * Query mode: ProcessIOMemPriority.exe query -pid pid")
TEXT("\n")
TEXT("\n    Retrieves the priority info for process <pid>.")
TEXT("\n")
TEXT("\n    Return %ERRORLEVEL%:")
TEXT("\n    0:         success")
TEXT("\n    fvalue:    error performing request")
TEXT("\n")
TEXT("\n    example: ProcessIOMemPriority.exe query -pid 1234")
TEXT("\n    Query current cpu, in/out and memory priorities of process with decimal PID value 1234.")
TEXT("\n")
TEXT("\n  * Set mode: ProcessIOMemPriority.exe set -pid pid { [-preset pres] [-cpupriority cpu] [-inoutpriority io] [-mempriority mem] }")
TEXT("\n")
TEXT("\n    Sets the priority of process <pid>.")
TEXT("\n    See 'Presets and priorities' section for supported values.")
TEXT("\n")
TEXT("\n    Return %ERRORLEVEL%:")
TEXT("\n    0:         success")
TEXT("\n    fvalue:    error change priority")
TEXT("\n")
TEXT("\n    example: ProcessIOMemPriority.exe set -pid 1234 -preset low -inoutpriority high")
TEXT("\n    Set cpu and memory priorities as <low> by preset, but in/out priority as <high> for the process with decimal PID value 1234.")
TEXT("\n    If You want to mix preset and custom priorities, You must use option -preset first before")
TEXT("\n    any custom priorities (-cpupriority, -inoutpriority and -mempriority)!")
TEXT("\n")
TEXT("\n  * Start mode: ProcessIOMemPriority.exe start { [-preset pres] [-cpupriority cpu] [-inoutpriority io] [-mempriority mem] } [-nowait] [-wd workingdirectory] -cmd <command> [args]")
TEXT("\n")
TEXT("\n    Run command line <command> [args] with priority.")
TEXT("\n    See 'Presets and priorities' section for supported values.")
TEXT("\n")
TEXT("\n    -nowait    - do not wait for the running child process to complete.")
TEXT("\n    -wd        - set the working directory for the child process,")
TEXT("\n               if the option is not specified, the working directory of the current process is inherited.")
TEXT("\n")
TEXT("\n    Return %ERRORLEVEL% if option -nowait used:")
TEXT("\n    PID:       child process ID")
TEXT("\n    fvalue:    error creating process or change priority")
TEXT("\n    Return %ERRORLEVEL% if option -nowait NOT used:")
TEXT("\n    exitcode:  child process exit code")
TEXT("\n    fvalue:    error creating process or change priority")
TEXT("\n")
TEXT("\n    example: ProcessIOMemPriority.exe start -inoutpriority high -wd c:\\windows -cmd wstunnel.exe -v --udp --udpTimeoutSec -1 -L 8043:127.0.0.1:51820 wss://wss.domain.com --upgradePathPrefix siteprefix")
TEXT("\n    Runs wstunnel.exe with some parameters with in/out priority <high> but don't change other priorities and wait until the wstunnel is stopped.")
TEXT("\n")
TEXT("\n  * Presets and priorities:")
TEXT("\n")
TEXT("\n    Windows application has only 2 memory priority values: Low and Normal.")
TEXT("\n    Windows application has 3 in/out priority values: Low, Normal and High.")
TEXT("\n    Windows application has 6 CPU priority values: Idle, BelowNormal, Normal, AboveNormal, High and Realtime.")
TEXT("\n    Windows creates a new user process with all priorities set to Normal by default.")
TEXT("\n")
TEXT("\n    Preset      CPU             In/Out     Memory")
TEXT("\n    low    ->   Below Normal    Low        Low")
TEXT("\n    normal ->   Normal          Normal     Normal")
TEXT("\n    high   ->   High            High       Normal")
TEXT("\n")
);
}

LPCTSTR CpuPriorityName(const DWORD PriorCPU);
LPCTSTR MemPriorityName(const DWORD PriorMem);
LPCTSTR InOutPriorityName(const DWORD PriorInOut);
LPCTSTR ActionName(const DWORD Action);
void clsPriorityOptions::DebugPrint()
{
	_tprintf(
TEXT("\n")
TEXT("options:\n")
TEXT("CPU:                 %s (%i)\n")
TEXT("Memory:              %s (%i)\n")
TEXT("In/Out:              %s (%i)\n")
TEXT("mode:                %s (%i)\n")
TEXT("PID:                 %i\n")
TEXT("command:             [%s]\n")
TEXT("WorkingDirectory:    %s\n")
TEXT("NoWait:              %s\n")
TEXT("ErrString:           %s\n")
TEXT("FailValue:           %i\n")
,
CpuPriorityName(CPU), CPU,
MemPriorityName(Memory), Memory,
InOutPriorityName(InOut), InOut,
ActionName(Action), Action,
TargetPid,
CmdLine,
WorkingDirectory,
NoWait? TEXT("yes"): TEXT("no"),
ErrStr,
FailSpec);
}


// ************************************************************************************************************************************
// ******************************************************** Common Subroutines ********************************************************
// ************************************************************************************************************************************



bool LoadNtdllLib()
{
	HMODULE ntdll = LoadLibrary(TEXT("ntdll.dll"));

	// locate the functions for enable privileges

	NtOpenProcessToken = (NtOpenProcessTokenFn)GetProcAddress(ntdll, "NtOpenProcessToken");
	NtAdjustPrivilegesToken = (NtAdjustPrivilegesTokenFn)GetProcAddress(ntdll, "NtAdjustPrivilegesToken");
	NtClose = (NtCloseFn)GetProcAddress(ntdll, "NtClose");

	// locate the functions for querying/setting memory and IO priority

	NtQueryInformationProcess = (NtQueryInformationProcessFn)GetProcAddress(ntdll, "NtQueryInformationProcess");
	NtSetInformationProcess = (NtSetInformationProcessFn)GetProcAddress(ntdll, "NtSetInformationProcess");

	if (!NtOpenProcessToken) { _tprintf(TEXT("Warning: failed to load NtOpenProcessToken from ntdll.dll\n")); }
	if (!NtAdjustPrivilegesToken) { _tprintf(TEXT("Warning: failed to load NtAdjustPrivilegesToken from ntdll.dll\n")); }
	if (!NtClose) { _tprintf(TEXT("Warning: failed to load NtClose from ntdll.dll\n")); }

	if (!NtQueryInformationProcess) { _tprintf(TEXT("Failed to load NtQueryInformationProcess from ntdll.dll\n")); return true; }
	if (!NtSetInformationProcess) { _tprintf(TEXT("Failed to load NtSetInformationProcess from ntdll.dll\n")); return true; }

	// When running the utility from the SYSTEM, it is not necessary to use NtOpenProcessToken-NtAdjustPrivilegesToken-NtClose

	return false;
} // LoadNtdllLib



VOID PhpEnablePrivileges(VOID) // KSystemInformer\main.c
{
    HANDLE tokenHandle;

	if (!NtOpenProcessToken || !NtAdjustPrivilegesToken || !NtClose) return;
	
	if (!NtOpenProcessToken(NtCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &tokenHandle))
    {
        CHAR privilegesBuffer[FIELD_OFFSET(TOKEN_PRIVILEGES, Privileges) + sizeof(LUID_AND_ATTRIBUTES) * 8];
        PTOKEN_PRIVILEGES privileges;

        privileges = (PTOKEN_PRIVILEGES)privilegesBuffer;
        privileges->PrivilegeCount = 8;

        for (DWORD i = 0; i < privileges->PrivilegeCount; i++)
        {
            privileges->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
            privileges->Privileges[i].Luid.HighPart = 0;
        }

        privileges->Privileges[0].Luid.LowPart = SE_DEBUG_PRIVILEGE;
        privileges->Privileges[1].Luid.LowPart = SE_INC_BASE_PRIORITY_PRIVILEGE;
        privileges->Privileges[2].Luid.LowPart = SE_INC_WORKING_SET_PRIVILEGE;
        privileges->Privileges[3].Luid.LowPart = SE_LOAD_DRIVER_PRIVILEGE;
        privileges->Privileges[4].Luid.LowPart = SE_PROF_SINGLE_PROCESS_PRIVILEGE;
        privileges->Privileges[5].Luid.LowPart = SE_RESTORE_PRIVILEGE;
        privileges->Privileges[6].Luid.LowPart = SE_SHUTDOWN_PRIVILEGE;
        privileges->Privileges[7].Luid.LowPart = SE_TAKE_OWNERSHIP_PRIVILEGE;

        NtAdjustPrivilegesToken(tokenHandle, FALSE, privileges, 0, NULL, NULL);

        NtClose(tokenHandle);
    }
} // PhpEnablePrivileges



  // *****************************************************************************************************************************
  // ******************************************************** Subroutines ********************************************************
  // *****************************************************************************************************************************



LPCTSTR CpuPriorityName(const DWORD PriorCPU)
{
	switch (PriorCPU)
	{
	case NORMAL_PRIORITY_CLASS:
		return TEXT("Normal (Default)");
	case REALTIME_PRIORITY_CLASS:
		return TEXT("Realtime");
	case HIGH_PRIORITY_CLASS:
		return TEXT("High");
	case ABOVE_NORMAL_PRIORITY_CLASS:
		return TEXT("Above normal");
	case BELOW_NORMAL_PRIORITY_CLASS:
		return TEXT("Below normal");
	case IDLE_PRIORITY_CLASS:
		return TEXT("Idle");
	default:
		return AUNKNOWN;
	}
}

LPCTSTR MemPriorityName(const DWORD PriorMem)
{
	switch (PriorMem)
	{
	case MEMORY_PRIORITY_LOWEST:
		return TEXT("Lowest");
	case MEMORY_PRIORITY_VERY_LOW:
		return TEXT("Very low");
	case MEMORY_PRIORITY_LOW:
		return TEXT("Low");
	case MEMORY_PRIORITY_MEDIUM:
		return TEXT("Medium");
	case MEMORY_PRIORITY_BELOW_NORMAL:
		return TEXT("Below normal");
	case MEMORY_PRIORITY_NORMAL:
		return TEXT("Normal (Default)");
	case MEMORY_PRIORITY_ABOVE_NORMAL:
		return TEXT("Above normal");
	case MEMORY_PRIORITY_HIGH:
		return TEXT("High");
	default:
		return AUNKNOWN;
	}
}

LPCTSTR InOutPriorityName(const DWORD PriorInOut)
{
	switch (PriorInOut)
	{
	case Priority::IoPriorityNormal:
		return TEXT("Normal (Default)");
	case Priority::IoPriorityLow:
		return TEXT("Low");
	case Priority::IoPriorityHigh:
		return TEXT("High");
	default:
		return AUNKNOWN;
	}
}

LPCTSTR ActionName(const DWORD Action)
{
	switch (Action)
	{
	case Action::Query:
		return AQUERY;
	case Action::Set:
		return ASET;
	case Action::Start:
		return ASTART;
	case Action::PrintHelp:
		return AHELP;
	default:
		return AUNKNOWN;
	}
}



bool SetPriority(const DWORD TargetPID, const DWORD PriorCPU, const DWORD PriorMem, const DWORD PriorInOut)
{
	// return = false	: OK
	// return = true	: FAIL

	HANDLE target = OpenProcess(PROCESS_SET_INFORMATION, false, TargetPID);
	if (!target)
	{
		_tprintf(TEXT("Failed to open process %u: [0x%X]\n"), TargetPID, GetLastError());
		return true;
	}

	if (PriorCPU != Priority::DontChangePriority)
	{
		// set the CPU priority
		if (!SetPriorityClass(target, PriorCPU))
		{
			_tprintf(TEXT("SetPriorityClass (CPU priority) failed: [0x%X]\n"), GetLastError());
			CloseHandle(target);
			return true;
		}
		_tprintf(TEXT("CPU priority for PID=%u set to '%s'\n"), TargetPID, CpuPriorityName(PriorCPU));
	}

	NTSTATUS result;

	if (PriorMem != Priority::DontChangePriority)
	{
		// set the memory priority
		result = NtSetInformationProcess(target, Priority::ProcessInformationMemoryPriority, &PriorMem, sizeof(PriorMem));
		if (result != 0)
		{
			_tprintf(TEXT("NtSetInformationProcess (Memory priority) failed: [0x%X]\n"), result);
			CloseHandle(target);
			return true;
		}
		_tprintf(TEXT("Memory priority for PID=%u set to '%s'\n"), TargetPID, MemPriorityName(PriorMem));
	}

	if (PriorInOut != Priority::DontChangePriority)
	{
		// set the IO priority
		result = NtSetInformationProcess(target, Priority::ProcessInformationInOutPriority, &PriorInOut, sizeof(PriorInOut));
		if (result != 0)
		{
			_tprintf(TEXT("NtSetInformationProcess (In/Out priority) failed: [0x%X]\n"), result);
			CloseHandle(target);
			return true;
			// ERROR_PRIVILEGE_NOT_HELD,               /* c0000061 (STATUS_PRIVILEGE_NOT_HELD) */ "A required privilege is not held by the client."
		}
		_tprintf(TEXT("In/Out priority for PID=%u set to '%s'\n"), TargetPID, InOutPriorityName(PriorInOut));
	}

	CloseHandle(target);
	return false;
}



DWORD StartProcPriority(DWORD PriorCPU, const DWORD PriorMem, const DWORD PriorInOut, LPTSTR CmdLine, LPCTSTR WorkingDirectory, const bool NoWait, const DWORD DefaultFailVal)
{
	bool failing = false;
	_tprintf(TEXT("RUN [%s]\n"), CmdLine);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	if (PriorCPU == Priority::DontChangePriority) PriorCPU = NORMAL_PRIORITY_CLASS;

	// fix:
	// if bInheritHandles=TRUE, -nowait option not working in batch like:
	// FOR /F "usebackq tokens=2 delims=[]" %%A IN (`ProcessIOMemPriority.exe start -nowait -cpupriority low -cmd notepad.exe`) DO SET "childpid=%%A"
	// note:
	// Windows 7: STD_INPUT_HANDLE, STD_OUTPUT_HANDLE, and STD_ERROR_HANDLE are inherited, even when the parameter is FALSE.
	// (https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa)

	if (!CreateProcess(NULL,   // No module name (use command line)
		CmdLine,		// Command line
		NULL,           // lpProcessAttributes
		NULL,           // lpThreadAttributes
		FALSE,			// Descriptors are inherited here. Inherited descriptors have the same values and access rights as the originals.
		PriorCPU | CREATE_SUSPENDED, // dwCreationFlags
		NULL,           // Use parent's environment block
		WorkingDirectory, // NULL if use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)			// Pointer to PROCESS_INFORMATION structure
		)
	{
		_tprintf(TEXT("CreateProcess failed [0x%X].\n"), GetLastError());
		return DefaultFailVal;
	}
	_tprintf(TEXT("Create Process with PID [%u] and CPU priority '%s'\n"), pi.dwProcessId, CpuPriorityName(PriorCPU));
	// _tprintf(TEXT("Process handle=%X\n"), pi.hProcess); DEBUG

	NTSTATUS result;

	// after calling PhpEnablePrivileges, it is not necessary to use NtOpenProcess(PROCESS_SET_INFORMATION)
	// OpenProcess(PROCESS_SET_INFORMATION) is also not needed here, pi.hProcess is sufficient

	if (PriorMem != Priority::DontChangePriority)
	{
		// set the memory priority
		result = NtSetInformationProcess(pi.hProcess, Priority::ProcessInformationMemoryPriority, &PriorMem, sizeof(PriorMem));
		if (result)
		{
			_tprintf(TEXT("NtSetInformationProcess (Memory priority) failed: [0x%X]\n"), result);
			failing = true; // don't exit, try to continue
		}
		_tprintf(TEXT("Memory priority set to '%s'\n"), MemPriorityName(PriorMem));
	}

	if (PriorInOut != Priority::DontChangePriority)
	{
		// set the IO priority
		result = NtSetInformationProcess(pi.hProcess, Priority::ProcessInformationInOutPriority, &PriorInOut, sizeof(PriorInOut));
		if (result)
		{
			_tprintf(TEXT("NtSetInformationProcess (In/Out priority) failed: [0x%X]\n"), result);
			failing = true; // don't exit, try to continue
			// ERROR_PRIVILEGE_NOT_HELD,               /* c0000061 (STATUS_PRIVILEGE_NOT_HELD) */ "A required privilege is not held by the client."
		}
		_tprintf(TEXT("In/Out priority set to '%s'\n"), InOutPriorityName(PriorInOut));
	}

	ResumeThread (pi.hThread);

	if (NoWait) return failing ? DefaultFailVal : pi.dwProcessId;
	// else
	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD ChildExitCode;
	if (!GetExitCodeProcess(pi.hProcess, (&ChildExitCode)))
	{
		_tprintf(TEXT("GetExitCodeProcess failed [0x%X].\n"), GetLastError());
		failing = true; // don't exit, try to continue
	}

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return failing ? DefaultFailVal : ChildExitCode;

// https://github.com/reactos/wine/blob/master/dlls/ntdll/error.c
// ERROR_INVALID_PARAMETER,                /* c0000003 (STATUS_INVALID_INFO_CLASS) */ i.e. not support in Windows XP
// ERROR_PRIVILEGE_NOT_HELD,               /* c0000061 (STATUS_PRIVILEGE_NOT_HELD) */ "A required privilege is not held by the client."

}



bool QueryPriority(DWORD TargetPid)
{
	bool failing = false;

	HANDLE target = OpenProcess(PROCESS_QUERY_INFORMATION, false, TargetPid);
	if (!target)
	{
		_tprintf(TEXT("Failed to open process %u: [0x%X]\n"), TargetPid, GetLastError());
		return true;
	}

	// find the CPU priority
	DWORD PriorCPU = GetPriorityClass(target);
	if (PriorCPU == 0)
	{
		PriorCPU = Priority::DontChangePriority;
		_tprintf(TEXT("GetPriorityClass (CPU Priority) failed: [0x%X]\n"), GetLastError());
		failing = true; // don't exit, try to continue
	}

	NTSTATUS result;
	ULONG len;

	// find the memory priority
	DWORD PriorMem;
	result = NtQueryInformationProcess(target, Priority::ProcessInformationMemoryPriority, &PriorMem, sizeof(PriorMem), &len);
	if (result != 0 || len != sizeof(PriorMem))
	{
		PriorMem = Priority::DontChangePriority;
		_tprintf(TEXT("NtQueryInformationProcess (Memory Priority) failed: [0x%X]\n"), result);
		failing = true; // don't exit, try to continue
	}

	// find the IO priority
	DWORD PriorInOut;
	result = NtQueryInformationProcess(target, Priority::ProcessInformationInOutPriority, &PriorInOut, sizeof(PriorInOut), &len);
	if (result != 0 || len != sizeof(PriorInOut))
	{
		PriorInOut = Priority::DontChangePriority;
		_tprintf(TEXT("NtQueryInformationProcess (In/Out Priority) failed: [0x%X]\n"), result);
		failing = true; // don't exit, try to continue
	}
	CloseHandle(target);

	_tprintf(TEXT("Query info for PID [%u]. CPU priority [%s], Memory priority [%s], IO priority [%s].\n"),
		TargetPid, CpuPriorityName(PriorCPU), MemPriorityName(PriorMem), InOutPriorityName(PriorInOut));
	return failing;
}

int _tmain(int argc, LPCTSTR argv[])
{
	DWORD exitcode;
	LPCTSTR	ErrorStr;
	clsPriorityOptions PriorOpt;

	ErrorStr = PriorOpt.ParseArgs(argc, argv);
	//PriorOpt.DebugPrint();
	if (ErrorStr)
	{
		printheader();
		_putts(ErrorStr);
		_putts(TEXT("run ProcessIOMemPriority.exe /? to get help.\n"));
		return -1;
	}

	printheader();

	if (PriorOpt.Action == Action::PrintHelp)
	{
		PrintHelp();
		return 0;
	}

	if (LoadNtdllLib()) return PriorOpt.FailSpec;
	PhpEnablePrivileges();

	if (PriorOpt.Action == Action::Query)
	{
		if (QueryPriority(PriorOpt.TargetPid)) exitcode = PriorOpt.FailSpec; else exitcode = 0;
	}
	else if (PriorOpt.Action == Action::Set)
	{
		if (SetPriority(PriorOpt.TargetPid, PriorOpt.CPU, PriorOpt.Memory, PriorOpt.InOut)) exitcode = PriorOpt.FailSpec; else exitcode = 0;
	}
	else if (PriorOpt.Action == Action::Start)
	{
		exitcode = StartProcPriority(PriorOpt.CPU, PriorOpt.Memory, PriorOpt.InOut, PriorOpt.CmdLine, PriorOpt.WorkingDirectory, PriorOpt.NoWait, PriorOpt.FailSpec);
	}
	else
	{
		_tprintf(TEXT("Unknown action: 0x%X\n"), PriorOpt.Action);
		exitcode = PriorOpt.FailSpec;
	}
	return exitcode;
}
