#include "pch.h"

// different const qualifiers
#pragma warning( disable : 4090)

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
    piInfo.pszFunctions = (LPTSTR)
        L"@hashnew,@hashfree,@hashget,@hashput,@hashdel,@hashclear,@hashcount,"
        L"hashentries";
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

#define MAX_HANDLE_LENGTH 21 // 20 characters + null

#ifndef TCCHM_DEBUG
    #define TCCHM_DEBUG 0
#endif    

#if TCCHM_DEBUG
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

static void entry_free(void *item) {
    const struct entry *entry = item;
    free(entry->key);  // there's only one allocated block - see f_hashput()
}

// Stores a handle (arbitrary string uniquely identifying the map) in 'dest'.
// 'dest' should be large enough to hold at least MAX_HANDLE_LENGTH characters.
// Returns the number of characters written (not counting the NULL)
static int writeHandleString(struct map *map, LPTSTR dest) {
    return swprintf(dest, MAX_HANDLE_LENGTH, L"%p", map);
}

// Parses 'handleStr' and returns a pointer to the struct map represented by the handle
// 'length' indicates the number of characters in 'handleStr' the function will consider.
static struct map * parseHandle(LPTSTR handleStr, size_t length) {
    struct map *map = NULL;
    _snwscanf(handleStr, length, L"%p", &map);
    return map;
}

// Usage: %@hashnew[optional-delim]
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
        wprintf(L"Usage: %%@hashget[handle,key[<delimiter>default_val]]\n");
        return -1;
    }
    struct map *map = parseHandle(paramStr, pcomma - paramStr);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    wchar_t *key = pcomma + 1;
    wchar_t *defaultVal = L"";
    wchar_t *pdelim = wcsstr(key, map->delimiter);
    if (pdelim) {  // user supplied a default value
        defaultVal = pdelim + wcslen(map->delimiter);
        *pdelim = L'\0'; // null-terminate key
    }
    struct entry *entry = hashmap_get(map->hashmap, &(struct entry){ .key=key });
    if (entry)
        wcscpy(paramStr, entry->value);
    else
        wcscpy(paramStr, defaultVal);
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

PLUGIN_API INT WINAPI f_hashdel(LPTSTR paramStr) {
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr) {
        wprintf(L"Usage: %%@hashdel[handle,key]\n");
        return -1;
    }
    struct map *map = parseHandle(paramStr, pcomma - paramStr);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    wchar_t *key = pcomma + 1;
    struct entry *entry = hashmap_delete(map->hashmap, &(struct entry){ .key=key });
    if (entry) {
        wcscpy(paramStr, entry->value);
        entry_free(entry);
    } else {
        paramStr[0] = L'\0';
    }
    return 0;
}

PLUGIN_API INT WINAPI f_hashclear(LPTSTR paramStr) {
    size_t len = wcslen(paramStr);
    if (len == 0) {
        wprintf(L"Usage: %%@hashclear[handle]\n");
        return -1;
    }
    struct map *map = parseHandle(paramStr, len);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    hashmap_clear(map->hashmap, true);
    return 0;
}

PLUGIN_API INT WINAPI f_hashcount(LPTSTR paramStr) {
    size_t len = wcslen(paramStr);
    if (len == 0) {
        wprintf(L"Usage: %%@hashcount[handle]\n");
        return -1;
    }
    struct map *map = parseHandle(paramStr, len);
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    unsigned int count = (unsigned int) hashmap_count(map->hashmap);
    _ultow_s(count, paramStr, 64, 10);
    return 0;
}

// Used by hashentries()
bool entry_iter_print_entry(const void *item, void *udata);

struct printEntryParams {
    wchar_t *delimiter;
    bool printKey;
    bool printVal;
};

/*
 * Prints a list of hash entries to stdout, one per line.
 */
PLUGIN_API INT WINAPI hashentries(LPTSTR argStr) {
    while (iswspace((wint_t) *argStr)) argStr++;  // Trim leading space
    if (*argStr == L'\0')   // to avoid special behavior with CommandLineToArgvW()
        goto argError;      //    when passed an empty string
    
    int numArgs;
    bool argError = false;
    WCHAR handle[MAX_HANDLE_LENGTH] = L"";
    bool printKey = true, printVal = true;

    LPWSTR *argv = CommandLineToArgvW(argStr, &numArgs);
    if (argv == NULL) goto argError;
    for (int i = 0; i < numArgs; i++) {
        size_t arglen = wcslen(argv[i]);
        if (arglen == 0) {
            argError = true;
        } else if (argv[i][0] == L'/') {
            if (arglen == 2) {
                switch (towupper(argv[i][1])) {
                case L'K':
                    printKey = true; printVal = false;
                    break;
                case L'V':
                    printKey = false; printVal = true;
                    break;
                default:
                    fwprintf(stderr, L"Unrecognized flag: %s\n", argv[i]);
                    argError = true;
                }
            } else {
                argError = true;
            }
        } else if (arglen < MAX_HANDLE_LENGTH) {
            wcscpy(handle, argv[i]);
        }
    }
    if (LocalFree(argv) != NULL)
        fputws(L"hashmap: LocalFree failed\n", stderr);
    if (argError || !wcslen(handle))
        goto argError;
    struct map *map = parseHandle(handle, wcslen(handle));
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
    } else {
        hashmap_scan(map->hashmap, entry_iter_print_entry, &(struct printEntryParams)
                { .delimiter = map->delimiter, .printKey = printKey, .printVal = printVal});
    }
    return 0;

  argError:
    _putws(L"Usage: hashentries [/K | /V] <handle>\n"
           L"\n"
           L"    /K = only print the keys\n"
           L"    /V = only print the values"
           );
    return -1;
}

static bool entry_iter_print_entry(const void *item, void *udata) {
    const struct entry *entry = item;
    const struct printEntryParams *params = udata;
    wchar_t *delim = params->delimiter;
    if (params->printKey)
        fputws(entry->key, stdout);
    if (params->printKey && params->printVal)
        fputws(delim, stdout);
    if (params->printVal)
        fputws(entry->value, stdout);
    fputwc(L'\n', stdout);
    return true;
}

