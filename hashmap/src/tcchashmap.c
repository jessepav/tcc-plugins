#include "pch.h"

#define PLUGIN_MAIN

#include "PlugIn.h"
#include "TakeCmd.h"

#include "hashmap.h"

#define PLUGIN_API __declspec(dllexport)

#define STRBUF_SIZE 4096
#define ARGBUF_SIZE 1024
#define MAX_FUNCSTR_SIZE 32000   // leave a few hundred charcters to spare
#define MAX_PARAMS 20

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

    piInfo.pszDll = (LPTSTR)L"Hashmap";
    piInfo.pszAuthor = (LPTSTR)L"Jesse Pavel";
    piInfo.pszEmail = (LPTSTR)L"jpavel@alum.mit.edu";
    piInfo.pszWWW = (LPTSTR)L"www.illcode.com";
    piInfo.pszDescription = (LPTSTR)L"Hashmap for TCC";
    piInfo.pszFunctions = (LPTSTR)L"@hashnew,@hashfree";
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

// ===================================
// Here begins the meat of the plugin.
// ===================================

#define MAX_DELIMITER_LENGTH 8
#define DEFAULT_DELIMITER L"/"

struct map {
    struct hashmap *hashmap;
    wchar_t delimiter[MAX_DELIMITER_LENGTH + 1];  // string delimiting arguments in TCC %@function[] calls
};

struct entry {
    wchar_t *key;
    wchar_t *value;
};
    
static int entry_compare(const void *a, const void *b, void *udata) {
    const struct entry *ea = a;
    const struct entry *eb = b;
    return wcscmp(ea->key, eb->key);
}

static uint64_t entry_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct entry *entry = item;
    return hashmap_sip(entry->key, wcslen(entry->key)*sizeof(wchar_t), seed0, seed1);
}

bool entry_iter(const void *item, void *udata) {
    const struct entry *entry = item;
    // Empty for now
    return true;  // false will stop iteration
}

// Stores a handle (arbitrary string uniquely identifying the map) in 'dest'.
// 'dest' should be large enough to hold at least 20 characters.
static void getHandle(struct map *map, LPTSTR dest) {
    swprintf(dest, 20, L"%p", map);
}

static struct map * parseHandle(LPTSTR handleStr) {
    struct map *map = NULL;
    swscanf(handleStr, L"%p", &map);
    return map;
}

PLUGIN_API INT WINAPI f_hashnew(LPTSTR paramStr) {
    struct map *map = malloc(sizeof(struct map));
    map->hashmap = hashmap_new(sizeof(struct entry), 0, 0, 0,
                               entry_hash, entry_compare, NULL, NULL);
    wchar_t *delimiter = DEFAULT_DELIMITER;
    wcsncpy(map->delimiter, delimiter, MAX_DELIMITER_LENGTH);
    // It may be the case, if the length of delimiter >= MAX_DELIMITER_LENGTH,
    // that a NULL is not appended, and so we terminate it manually.
    map->delimiter[MAX_DELIMITER_LENGTH] = L'\0';
    getHandle(map, paramStr);
    return 0;
}

PLUGIN_API INT WINAPI f_hashfree(LPTSTR paramStr) {
    struct map *map = parseHandle(paramStr);
    Printf(L"Freeing the map at address %p", map);
    hashmap_free(map->hashmap);
    free(map);
    return 0;
}

/* ================== Unused =================== */

PLUGIN_API INT WINAPI f_hashmap(LPTSTR lpszString) {
    UINT idx = 0, nParamIdx;
    UINT nParams = 0;
    idx += swprintf_s(strbuf + idx, STRBUF_SIZE - idx, L"Parameter string: <<%s>>\n", lpszString);
    idx += swprintf_s(strbuf + idx, STRBUF_SIZE - idx, L"Number of parameters: ");
    nParamIdx = idx;
    idx += 3;  // reserve space to backpatch nParams (2 digits + newline)
    for (int i = 0; i < MAX_PARAMS; i++) {
        if (NthArgument(lpszString, i, argbuf, NULL) != NULL) {
            StripEnclosingQuotes(argbuf);
            EscapeLine(argbuf);
            idx += swprintf_s(strbuf + idx, STRBUF_SIZE - idx,  L"Arg %d: <<%s>>\n", i + 1, argbuf);
            nParams++;
        } else {
            break;
        }
    }
    swprintf(argbuf, 4, L"%-2d\n", nParams);  // we use 4 because of the NULL terminator
    wmemcpy(strbuf + nParamIdx, argbuf, 3);
    strbuf[idx] = L'\0'; // if nParams == 0, the string might not be NULL-terminated
    fputws(strbuf, stdout);
    wcscpy_s(lpszString, MAX_FUNCSTR_SIZE, L"Swizzle!");
    return 0;
}
