#include "pch.h"

#define PLUGIN_MAIN

#include "PlugIn.h"
#include "TakeCmd.h"

#include "swizzle.h"

WCHAR *strbuf;
WCHAR *argbuf;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

PLUGIN_API LPPLUGININFO WINAPI GetPluginInfo(HMODULE hModule) {
    static PLUGININFO piInfo;

    piInfo.pszDll = (LPTSTR)L"Swizzle";
    piInfo.pszAuthor = (LPTSTR)L"Jesse Pavel";
    piInfo.pszEmail = (LPTSTR)L"jpavel@alum.mit.edu";
    piInfo.pszWWW = (LPTSTR)L"www.illcode.com";
    piInfo.pszDescription = (LPTSTR)L"All-Purpose Swizzling";
    piInfo.pszFunctions = (LPTSTR)L"@swizzle";
    piInfo.nMajor = 1;
    piInfo.nMinor = 0;
    piInfo.nBuild = 1;

    return &piInfo;
}

PLUGIN_API BOOL WINAPI InitializePlugin(void) {
    strbuf = AllocMem(sizeof(WCHAR) * STRBUF_SIZE);
    argbuf = AllocMem(sizeof(WCHAR) * ARGBUF_SIZE);
    return 0;
}

PLUGIN_API BOOL WINAPI ShutdownPlugin(BOOL bEndProcess) {
    FreeMem(argbuf);
    FreeMem(strbuf);
    return 0;
}
