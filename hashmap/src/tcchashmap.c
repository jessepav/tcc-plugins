#include "pch.h"

// different const qualifiers
#pragma warning( disable : 4090)

#define PLUGIN_MAIN

#include "PlugIn.h"
#include "TakeCmd.h"

#include "hashmap.h"

#define PLUGIN_API __declspec(dllexport)

// =============================================

#define MAX_DELIMITER_LENGTH 8
#define DEFAULT_DELIMITER L"/"

#define MAX_HANDLE_LENGTH 21 // 20 characters + null
#define DEFAULT_HANDLE_CAPACITY 16
#define MAX_HANDLE_CAPACITY 1024
#define INVALID_HANDLE UINT_MAX

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

// ==============================================
// Data Structures and Global Variables
// ==============================================

struct map {
    struct hashmap *hashmap;
    wchar_t delimiter[MAX_DELIMITER_LENGTH + 1];  // string delimiting arguments in TCC %@function[] calls
};

struct entry {
    wchar_t *key;
    wchar_t *value;
};
    
struct printEntryParams {
    wchar_t *delimiter;
    bool printKey;
    bool printVal;
};

// In our parlance, a "handle" is simply a number, starting at 0,
// that represents an index into an array of map pointers.

static unsigned int handleCapacity;  // the size of our arrays
static unsigned int nextHandleIdx;   // the index into availableHandles of the next available handle
static unsigned int *availableHandles; // array of handles
static struct map **mapPtrs; // map from handle to struct map pointers

// ==============================================
// Plugin Lifecycle functions
// ==============================================

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
        L"@hashnew,@hashfree,@hashdelim,@hashget,@hashput,@hashdel,@hashclear,@hashcount,"
        L"hashentries";
    piInfo.nMajor = 1;
    piInfo.nMinor = 0;
    piInfo.nBuild = 1;

    return &piInfo;
}

PLUGIN_API BOOL WINAPI InitializePlugin(void) {
    handleCapacity = DEFAULT_HANDLE_CAPACITY;
    nextHandleIdx = 0;
    availableHandles = malloc(sizeof(unsigned int) * handleCapacity);
    mapPtrs = malloc(sizeof(struct map *) * handleCapacity);
    for (unsigned int i = 0; i < handleCapacity; i++) {
        availableHandles[i] = i;
        mapPtrs[i] = NULL;
    }
    return 0;
}

PLUGIN_API BOOL WINAPI ShutdownPlugin(BOOL bEndProcess) {
    free(mapPtrs);
    free(availableHandles);
    return 0;
}

// ==============================================
// Utility functions
// ==============================================

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

/*
 * Returns a new handle, or INVALID_HANDLE if no more handles are available, or a
 * memory allocation error was encountered.
 */
static unsigned int checkoutHandle() {
    if (nextHandleIdx == handleCapacity) { // we need to grow our arrays
        if (handleCapacity == MAX_HANDLE_CAPACITY)
            return INVALID_HANDLE;
        handleCapacity *= 2;
        DEBUG_PRINTF(L"Growing handle arrays: new capacity = %u\n", handleCapacity);
        void *p = realloc(availableHandles, sizeof(unsigned int) * handleCapacity);
        void *q = realloc(mapPtrs, sizeof(struct map *) * handleCapacity);
        if (!p || !q)
            return INVALID_HANDLE;
        availableHandles = p;
        mapPtrs = q;
        for (unsigned int i = nextHandleIdx; i < handleCapacity; i++) {
            availableHandles[i] = i;
            mapPtrs[i] = NULL;
        }
    }
    return availableHandles[nextHandleIdx++];
}

static void releaseHandle(unsigned int handle) {
    if (nextHandleIdx > 0) {
        availableHandles[--nextHandleIdx] = handle;
        mapPtrs[handle] = NULL;
    }
}

// Stores a handle (arbitrary string uniquely identifying the map) in 'dest'.
// 'dest' should be large enough to hold at least MAX_HANDLE_LENGTH characters.
// Returns the number of characters written (not counting the NULL)
static int writeHandleString(unsigned int handle, LPTSTR dest) {
    return swprintf(dest, MAX_HANDLE_LENGTH, L"H%uU", handle);
}

// Parses 'handleStr' and, if successful, stores the parsed handle in *handle.
// 'length' indicates the number of characters in 'handleStr' the function will consider.
// Returns 'true' if the parse was successful, 'false' otherwise.
static bool parseHandle(LPTSTR handleStr, size_t length, unsigned int *handle) {
    unsigned int h;
    if (_snwscanf(handleStr, length, L"H%uU", &h) == 1 && h < handleCapacity) {
        *handle = h;
        return true;
    } else {
        return false;
    }
}

// Used by hashentries()
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

// ==============================================
// TCC %@[] functions
// ==============================================

// Usage: %@hashnew[optional-delim]
PLUGIN_API INT WINAPI f_hashnew(LPTSTR paramStr) {
    StripEnclosingQuotes(paramStr);
    size_t n = wcslen(paramStr);
    if (n > MAX_DELIMITER_LENGTH) {
        fwprintf(stderr, L"hashmap: delimiter length exceeds maximum allowed (%d)\n", MAX_DELIMITER_LENGTH);
        paramStr[0] = L'\0';
        return -1;
    }
    wchar_t *delimiter = n != 0 ? paramStr : DEFAULT_DELIMITER;
    unsigned int handle = checkoutHandle();
    if (handle == INVALID_HANDLE) {
        fwprintf(stderr, L"hashmap: unable to allocate new handle\n");
        paramStr[0] = L'\0';
        return -1;
    }
    struct map *map = malloc(sizeof(struct map));
    map->hashmap = hashmap_new(sizeof(struct entry), 0, 0, 0,
                               entry_hash, entry_compare, entry_free, NULL);
    wcscpy(map->delimiter, delimiter);
    mapPtrs[handle] = map;
    writeHandleString(handle, paramStr);
    return 0;
}

PLUGIN_API INT WINAPI f_hashfree(LPTSTR paramStr) {
    unsigned int handle;
    struct map *map = NULL;
    if (parseHandle(paramStr, wcslen(paramStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    paramStr[0] = L'\0';  // return an empty string
    hashmap_free(map->hashmap);
    _set_errno(0);
    free(map);
    if (errno != 0)
        _wperror(L"f_hashfree");
    releaseHandle(handle);
    return 0;
}

PLUGIN_API INT WINAPI f_hashdelim(LPTSTR paramStr) {
    unsigned int handle;
    struct map *map = NULL;
    size_t len = wcslen(paramStr);
    if (len == 0) {
        wprintf(L"Usage: %%@hashdelim[handle]\n");
        return -1;
    }
    if (parseHandle(paramStr, wcslen(paramStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    wcscpy(paramStr, map->delimiter);
    return 0;
}

PLUGIN_API INT WINAPI f_hashget(LPTSTR paramStr) {
    unsigned int handle;
    struct map *map = NULL;
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr) {
        wprintf(L"Usage: %%@hashget[handle,key[<delimiter>default_val]]\n");
        return -1;
    }
    if (parseHandle(paramStr, pcomma - paramStr, &handle))
        map = mapPtrs[handle];
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
    unsigned int handle;
    struct map *map = NULL;
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr)
        goto paramError;
    if (parseHandle(paramStr, pcomma - paramStr, &handle))
        map = mapPtrs[handle];
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
    unsigned int handle;
    struct map *map = NULL;
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (!pcomma || pcomma == paramStr) {
        wprintf(L"Usage: %%@hashdel[handle,key]\n");
        return -1;
    }
    if (parseHandle(paramStr, pcomma - paramStr, &handle))
        map = mapPtrs[handle];
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
    unsigned int handle;
    struct map *map = NULL;
    size_t len = wcslen(paramStr);
    if (len == 0) {
        wprintf(L"Usage: %%@hashclear[handle]\n");
        return -1;
    }
    if (parseHandle(paramStr, wcslen(paramStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    hashmap_clear(map->hashmap, true);
    return 0;
}

PLUGIN_API INT WINAPI f_hashcount(LPTSTR paramStr) {
    unsigned int handle;
    struct map *map = NULL;
    size_t len = wcslen(paramStr);
    if (len == 0) {
        wprintf(L"Usage: %%@hashcount[handle]\n");
        return -1;
    }
    if (parseHandle(paramStr, wcslen(paramStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    unsigned int count = (unsigned int) hashmap_count(map->hashmap);
    _ultow(count, paramStr, 10);
    return 0;
}

// ==============================================
// Commands
// ==============================================

/*
 * Prints a list of hash entries to stdout, one per line.
 */
PLUGIN_API INT WINAPI hashentries(LPTSTR argStr) {
    while (iswspace((wint_t) *argStr)) argStr++;  // Trim leading space
    if (*argStr == L'\0')   // to avoid special behavior with CommandLineToArgvW()
        goto argError;      //    when passed an empty string
    
    int numArgs;
    bool argError = false;
    WCHAR handleStr[MAX_HANDLE_LENGTH] = L"";
    bool printKey = true, printVal = true;
    unsigned int handle;
    struct map *map = NULL;

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
            wcscpy(handleStr, argv[i]);
        }
    }
    if (LocalFree(argv) != NULL)
        fputws(L"hashmap: LocalFree failed\n", stderr);
    if (argError || !wcslen(handleStr))
        goto argError;
    if (parseHandle(handleStr, wcslen(handleStr), &handle))
        map = mapPtrs[handle];
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

