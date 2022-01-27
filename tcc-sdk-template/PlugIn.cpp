// PlugIn.cpp - demonstration plugin for TCC 28
// Plugins should be linked with the /DYNAMICBASE (Use address space layout randomization) option

#define UNICODE 1
#define _UNICODE 1

#define _ATL_ALLOW_CHAR_UNSIGNED 1

#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#include "PlugIn.h"
#include "TakeCmd.h"


BOOL APIENTRY DllMain( HANDLE hModule, DWORD  dwReason, LPVOID lpReserved )
{
	UNREFERENCED_PARAMETER(lpReserved);
	UNREFERENCED_PARAMETER(hModule);
	switch( (int)dwReason )	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}


DLLExports LPPLUGININFO WINAPI GetPluginInfo( void )
{
	static PLUGININFO piInfo;

	piInfo.pszDll = (LPTSTR)L"Plugin";
	piInfo.pszAuthor = (LPTSTR)L"JP Software";
	piInfo.pszEmail = (LPTSTR)L"support@jpsoft.com";
	piInfo.pszWWW = (LPTSTR)L"https://jpsoft.com";
	piInfo.pszDescription = (LPTSTR)L"Plugin Demo";
	piInfo.pszFunctions = (LPTSTR)L"@reverse,_hello,remark,UNKNOWN_CMD,*key";
	piInfo.nMajor = 1;
	piInfo.nMinor = 0;
	piInfo.nBuild = 1;

	return &piInfo;
}


DLLExports BOOL WINAPI InitializePlugin( void )
{
	return 0;
}


DLLExports BOOL WINAPI ShutdownPlugin( BOOL bEndProcess )
{
	UNREFERENCED_PARAMETER(bEndProcess);
	return 0;
}


// this is a variable function called from TCC; it reverses the string argument
DLLExports INT WINAPI f_reverse( LPTSTR lpszString )
{
	if ( lpszString == NULL )
		return 1;

	_wcsrev( lpszString );
	return 0;
}


// this is an internal variable called from TCC
DLLExports INT WINAPI _hello( LPTSTR lpszString )
{
	if ( lpszString == NULL )
		return 1;

	lstrcpy( lpszString, L"Hello, PlugIn World!" );
	return 0;
}


// this is an internal command called from TCC
DLLExports INT WINAPI remark( LPTSTR lpszString )
{
	UNREFERENCED_PARAMETER(lpszString);
	wprintf( L"What a trivial sample PlugIn!\r\n" );
	return 0;
}


// this is an internal command called from TCC
DLLExports INT WINAPI UNKNOWN_CMD( LPTSTR lpszString )
{
	UNREFERENCED_PARAMETER(lpszString);
	wprintf( L"What the heck are you doing now?\r\n" );
	return 0;
}


// keystroke plugin - change a Ctrl-Home key into a "foo" string
DLLExports INT WINAPI key( LPKEYINFO ki )
{
	if ((ki->nKey == 0) && (ki->pszKey) && (lstrcmpi(ki->pszKey, _T("Ctrl-Home")) == 0)) {
		ki->fRedraw = 1;
		wsprintf(ki->pszLine, L"foo");
		ki->pszCurrent = ki->pszLine+3;
		ki->pszKey[0] = _T('\0');
	}

	return 0;
}
