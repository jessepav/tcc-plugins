#include "swizzle-pch.h"

#include "PlugIn.h"
#include "TakeCmd.h"

#define PLUGIN_API __declspec(dllexport)

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

PLUGIN_API LPPLUGININFO WINAPI GetPluginInfo(void) {
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
    return 0;
}

PLUGIN_API BOOL WINAPI ShutdownPlugin(BOOL bEndProcess) {
    return 0;
}

PLUGIN_API INT WINAPI f_swizzle(LPTSTR lpszString) {
    Sprintf(lpszString, L"Swizzle!");
    return 0;
}
