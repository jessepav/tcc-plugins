#include "pch.h"

// different const qualifiers
#pragma warning( disable : 4090)

#define PLUGIN_MAIN

#include "PlugIn.h"
#include "TakeCmd.h"

#include "hashmap.h"

#define PLUGIN_API __declspec(dllexport)

#define BUFSIZE 16384

static wchar_t *buf;  // scratch buffer used by various functions

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
    piInfo.pszFunctions = (LPTSTR)
        L"@hashnew,@hashfree,@hashget,@hashput";
    piInfo.nMajor = 1;
    piInfo.nMinor = 0;
    piInfo.nBuild = 1;

    return &piInfo;
}

PLUGIN_API BOOL WINAPI InitializePlugin(void) {
    buf = malloc((size_t) BUFSIZE * sizeof(wchar_t));
    return 0;
}

PLUGIN_API BOOL WINAPI ShutdownPlugin(BOOL bEndProcess) {
    free(buf);
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

static bool entry_iter(const void *item, void *udata) {
    const struct entry *entry = item;
    // Empty for now
    return true;  // false will stop iteration
}

static void entry_free(void *item) {
    const struct entry *entry = item;
    free(entry->key);  // there's only one allocated block - see f_hashput()
}

// Stores a handle (arbitrary string uniquely identifying the map) in 'dest'.
// 'dest' should be large enough to hold at least 20 characters.
// Returns the number of characters written (not counting the NULL)
static int writeHandleString(struct map *map, LPTSTR dest) {
    return swprintf(dest, 20, L"%p", map);
}

// Parses 'handleStr' and returns a pointer to the struct map represented by the handle
// 'length' indicates the number of characters in 'handleStr' the function will consider.
static struct map * parseHandle(LPTSTR handleStr, size_t length) {
    struct map *map = NULL;
    _snwscanf(handleStr, length, L"%p", &map);
    return map;
}

PLUGIN_API INT WINAPI f_hashnew(LPTSTR paramStr) {
    struct map *map = malloc(sizeof(struct map));
    map->hashmap = hashmap_new(sizeof(struct entry), 0, 0, 0,
                               entry_hash, entry_compare, entry_free, NULL);
    StripEnclosingQuotes(paramStr);
    size_t n = wcslen(paramStr);
    wchar_t *delimiter = n != 0 ? paramStr : DEFAULT_DELIMITER;
    wcsncpy(map->delimiter, delimiter, MAX_DELIMITER_LENGTH);
    // It may be the case, if the length of delimiter >= MAX_DELIMITER_LENGTH,
    // that a NULL is not appended, and so we terminate it manually.
    map->delimiter[MAX_DELIMITER_LENGTH] = L'\0';
    DEBUG_PRINTF(L"Allocated a map at address 0x%p with delimiter \"%s\"\n", map, map->delimiter);
    writeHandleString(map, paramStr);
    return 0;
}

PLUGIN_API INT WINAPI f_hashfree(LPTSTR paramStr) {
    struct map *map = parseHandle(paramStr, wcslen(paramStr));
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
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

PLUGIN_API INT WINAPI f_hashget(LPTSTR paramStr) {
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr) {
        wprintf(L"Usage: %%@hashget[handle,key]\n");
        return -1;
    }
    struct map *map = parseHandle(paramStr, pcomma - paramStr);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    wchar_t *key = pcomma + 1;
    DEBUG_PRINTF(L"map = %p, key = %s\n", map, key);
    struct entry *entry = hashmap_get(map->hashmap, &(struct entry){ .key=key });
    if (entry)
        wcscpy(paramStr, entry->value);
    else
        paramStr[0] = L'\0';  // empty value
    return 0;
}

PLUGIN_API INT WINAPI f_hashput(LPTSTR paramStr) {
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr)
        goto paramError;
    struct map *map = parseHandle(paramStr, pcomma - paramStr);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    wchar_t *key = pcomma + 1;
    wchar_t *pdelim = wcsstr(key, map->delimiter);
    if (!pdelim || pdelim == key)
        goto paramError;
    *pdelim = L'\0';  // null-terminate the key
    wchar_t *val = pdelim + wcslen(map->delimiter);
    size_t keylen = pdelim - key;
    size_t vallen = wcslen(val);
    struct entry entry;
    entry.key = malloc(sizeof(wchar_t) * (keylen + 1 + vallen + 1));  // +1's for terminating nulls
    wcscpy(entry.key, key);
    entry.value = entry.key + keylen + 1;
    wcscpy(entry.value, val);
    struct entry *oldEntry = hashmap_set(map->hashmap, &entry);
    if (oldEntry) {
        wcscpy(paramStr, oldEntry->value);
        entry_free(oldEntry);
    } else {
        paramStr[0] = L'\0';  // empty value
    }
    return 0;
    
  paramError:
    wprintf(L"Usage: %%@hashput[handle,key<delimiter>value]\n");
    return -1;
}
