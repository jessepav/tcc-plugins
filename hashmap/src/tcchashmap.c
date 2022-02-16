#include "pch.h"

#define PLUGIN_MAIN

#include "PlugIn.h"
#include "TakeCmd.h"

#include "hashmap.h"

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
    return 0;
}

PLUGIN_API BOOL WINAPI ShutdownPlugin(BOOL bEndProcess) {
    return 0;
}

// ===================================
// Here begins the meat of the plugin.
// ===================================

#define MAX_DELIMITER_LENGTH 8
#define DEFAULT_DELIMITER L"/"

#define DEBUG 1

#if DEBUG
    #define DEBUG_PRINTF(format, ...) fwprintf(stderr, format, __VA_ARGS__)
    #define DEBUG_PRINT(msg) fputws(msg, stderr)
#else
    #define DEBUG_PRINTF
    #define DEBUG_PRINT
#endif

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
    DEBUG_PRINTF(L"Allocated a map at address 0x%p with delimiter \"%s\"\n", map, map->delimiter);
    getHandle(map, paramStr);
    return 0;
}

PLUGIN_API INT WINAPI f_hashfree(LPTSTR paramStr) {
    struct map *map = parseHandle(paramStr);
    DEBUG_PRINTF(L"Freeing the map at address 0x%p\n", map);
    paramStr[0] = L'\0';  // return an empty string
    hashmap_free(map->hashmap);
    _set_errno(0);
    free(map);
    if (errno != 0) {
        _wperror(L"f_hashfree");
        return -1;
    } else {
        return 0;
    }
}
