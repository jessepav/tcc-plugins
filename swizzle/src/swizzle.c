#include "pch.h"

#include "PlugIn.h"
#include "TakeCmd.h"

#define PLUGIN_API __declspec(dllexport)

#define STRBUF_SIZE 4096
#define ARGBUF_SIZE 1024
#define MAX_FUNCSTR_SIZE 32000   // leave a few hundred charcters to spare

static WCHAR *strbuf;
static WCHAR *argbuf;

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

#define SAPPEND(sprint_func) \
    do { \
        n = sprint_func; \
        if (n == -1) { return 1; } else { idx += n; remaining -= n; } \
    } while (0)

PLUGIN_API INT WINAPI f_swizzle(LPTSTR lpszString) {
    int n, idx = 0, remaining = STRBUF_SIZE - 1;
    SAPPEND(swprintf_s(strbuf + idx, remaining, L"Parameter string: <<%s>>\n", lpszString));
    for (int i = 0; i < 20; i++) {
        if (NthArgument(lpszString, i, argbuf, NULL) != NULL) {
            StripEnclosingQuotes(argbuf);
            EscapeLine(argbuf);
            SAPPEND(swprintf_s(strbuf + idx, remaining,  L"Arg %d <<%s>>\n", i + 1, argbuf));
        } else {
            break;
        }
    }
    Printf(L"%s", strbuf);
    wcscpy_s(lpszString, MAX_FUNCSTR_SIZE, L"Swizzle!");
    return 0;
}
