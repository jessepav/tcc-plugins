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
#define HANDLE_FORMAT L"H%04uU"
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

// In our parlance a "handle" is simply a number, starting at 0,
// that represents an index into an array of map pointers.

static unsigned int handleCapacity;  // the size of our arrays
static unsigned int nextHandleIdx;   // the index into availableHandles of the next available handle
static unsigned int *availableHandles; // array of handles
static struct map **mapPtrs; // table from handle (i.e. index) to map pointers

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
        L"hashentries,hashfile,hashfreeall";
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
    // Okay, let's check for existing maps and free them
    int numFreed = 0;
    for (unsigned int i = 0; i < handleCapacity; i++) {
        if (mapPtrs[i] != NULL) {
            hashmap_free(mapPtrs[i]->hashmap);
            _set_errno(0);
            free(mapPtrs[i]);
            if (errno != 0)
                _wperror(L"hashmap shutdown");
            numFreed++;
        }
    }
    if (numFreed)    
        wprintf(L"hashmap: freed %d outstanding map(s)\n", numFreed);
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
    return swprintf(dest, MAX_HANDLE_LENGTH, HANDLE_FORMAT, handle);
}

// Parses 'handleStr' and, if successful, stores the parsed handle in *handle.
// 'length' indicates the number of characters in 'handleStr' the function will consider.
// Returns 'true' if the parse was successful, 'false' otherwise.
static bool parseHandle(LPTSTR handleStr, size_t length, unsigned int *handle) {
    unsigned int h;
    if (_snwscanf(handleStr, length, HANDLE_FORMAT, &h) == 1 && h < handleCapacity) {
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

// Used by hashfile()
static bool entry_iter_write_entry(const void *item, void *udata) {
    const struct entry *entry = item;
    const FILE *fp = udata;
    uint32_t keyLen = (uint32_t) wcslen(entry->key);   // in units of characters!
    uint32_t valueLen = (uint32_t) wcslen(entry->value);
    fwrite(&keyLen, 4, 1, fp);
    fwrite(&valueLen, 4, 1, fp);
    fwrite(entry->key, sizeof(wchar_t), keyLen, fp);
    fwrite(entry->value, sizeof(wchar_t), valueLen, fp);
    return true;
}

// ==============================================
// TCC %@[] functions
// ==============================================

// Usage: %@hashnew[[<capacity>]]
PLUGIN_API INT WINAPI f_hashnew(LPTSTR paramStr) {
    unsigned long capacity = 0;  // let hashmap.c use the default
    if (wcslen(paramStr) > 0) {    // user passed a capacity
        wchar_t *endptr;
        capacity = wcstoul(paramStr, &endptr, 10);
        if (*endptr != L'\0' || capacity == ULONG_MAX) {  // invalid capacity string
            fwprintf(stderr, L"Invalid capacity given\n");
            paramStr[0] = L'\0';
            return -1;
        }
    }
    unsigned int handle = checkoutHandle();
    if (handle == INVALID_HANDLE) {
        fwprintf(stderr, L"hashmap: unable to allocate new handle\n");
        paramStr[0] = L'\0';
        return -1;
    }
    struct map *map = malloc(sizeof(struct map));
    map->hashmap = hashmap_new(sizeof(struct entry), (size_t) capacity, 0, 0,
                               entry_hash, entry_compare, entry_free, NULL);
    wcscpy(map->delimiter, DEFAULT_DELIMITER);
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
    if (paramStr[0] == L'\0') {
        wprintf(L"Usage: %%@hashdelim[handle[,<delimiter>]\n");
        return -1;
    }
    wchar_t *newdelim = NULL;
    wchar_t *pcomma = wcschr(paramStr, L',');
    if (pcomma) { // we're setting a new delimiter
        *pcomma = L'\0';  // null-terminate handle
        newdelim = pcomma + 1;
        if (wcslen(newdelim) > MAX_DELIMITER_LENGTH) {
            fwprintf(stderr, L"hashmap: delimiter length exceeds maximum allowed (%d)\n", MAX_DELIMITER_LENGTH);
            paramStr[0] = L'\0';
            return -1;
        }
    }
    if (parseHandle(paramStr, wcslen(paramStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
        return -1;
    }
    if (newdelim)
        wcscpy(map->delimiter, newdelim);
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

#define HASHFILE_READ   0
#define HASHFILE_MERGE  1
#define HASHFILE_WRITE  2

// fopen() mode strings corresponding to each of the above command constants
static const wchar_t *hashfile_modes[] = {
    [HASHFILE_READ]  = L"rb", 
    [HASHFILE_MERGE] = L"rb",
    [HASHFILE_WRITE] = L"wb"
};

static const uint16_t hashfile_marker         = 0xABCD;
static const uint16_t hashfile_marker_swapped = 0xCDAB;    // detect wrong endianness

// macro used in the case HASHFILE_MERGE branch of hashfile()
#define HASHFILE_CHECK_READ(buffer, size, count, stream) \
            if (fread(buffer, size, count, stream) != count) goto readError

/*
 * Read/write map to/from a file.
 */
PLUGIN_API INT WINAPI hashfile(LPTSTR argStr) {
    LPWSTR *argv = NULL;
    int numArgs;
    wchar_t *handleStr;
    int command;
    wchar_t *filename;
    FILE *fp = NULL;

    bool success = false;

    while (iswspace((wint_t) *argStr)) argStr++;  // Trim leading space
    if (*argStr == L'\0')   // to avoid special behavior with CommandLineToArgvW()
        goto showHelp;      //    when passed an empty string
    
    argv = CommandLineToArgvW(argStr, &numArgs);
    if (argv == NULL || numArgs != 3)
        goto showHelp;

    // I declare variables here just in case we change our method of argument parsing later.
    wchar_t *handleArg, *commandArg, *filenameArg;
    handleArg = argv[0];
    commandArg = argv[1];
    filenameArg = argv[2];
    
    // Get the handle
    if (wcslen(handleArg) < MAX_HANDLE_LENGTH)
        handleStr = handleArg;
    else
        goto showHelp;
    // Then the command
    _wcsupr(commandArg);  // convert flag to uppercase
    if (wcscmp(commandArg, L"/R") == 0)
        command = HASHFILE_READ;
    else if (wcscmp(commandArg, L"/M") == 0)
        command = HASHFILE_MERGE;
    else if (wcscmp(commandArg, L"/W") == 0)
        command = HASHFILE_WRITE;
    else
        goto showHelp;
    // And the filename
    filename = filenameArg;

    unsigned int handle;
    struct map *map = NULL;

    if (parseHandle(handleStr, wcslen(handleStr), &handle))
        map = mapPtrs[handle];
    if (map == NULL) {
        wprintf(L"Hashmap: invalid handle\n");
    } else {
        fp = _wfopen(filename, hashfile_modes[command]);
        if (fp == NULL) {
            _wperror(L"hashfile");
            goto cleanup;
        }
        switch (command) {
        case HASHFILE_READ:
            hashmap_clear(map->hashmap, true);
            // fall through
        case HASHFILE_MERGE:
            {
                uint16_t marker = 0;
                uint32_t count = 0;
                bool needByteSwap;

                HASHFILE_CHECK_READ(&marker, 2, 1, fp);
                if (marker == hashfile_marker)
                    needByteSwap = false;
                else if (marker == hashfile_marker_swapped)
                    needByteSwap = true;
                else
                    goto readError;
                HASHFILE_CHECK_READ(&count, 4, 1, fp);
                for (unsigned int i = 0; i < count; i++) {
                    uint32_t keyLen, valueLen;
                    HASHFILE_CHECK_READ(&keyLen, 4, 1, fp);
                    HASHFILE_CHECK_READ(&valueLen, 4, 1, fp);
                    if (needByteSwap) {
                        keyLen = _byteswap_ulong(keyLen);
                        valueLen = _byteswap_ulong(valueLen);
                    }
                    wchar_t *buf = malloc(sizeof(wchar_t) * (keyLen + valueLen + 2));
                    HASHFILE_CHECK_READ(buf, sizeof(wchar_t), keyLen, fp);
                    buf[keyLen] = L'\0';
                    wchar_t *value = buf + keyLen + 1;
                    HASHFILE_CHECK_READ(value, sizeof(wchar_t), valueLen, fp);
                    value[valueLen] = L'\0';
                    if (needByteSwap) {
                        for (UINT p = 0; p < keyLen; p++)
                            buf[p] = _byteswap_ushort(buf[p]);
                        for (UINT p = 0; p < valueLen; p++)
                            value[p] = _byteswap_ushort(value[p]);
                    }
                    struct entry *oldEntry = hashmap_set(map->hashmap, &(struct entry){ .key = buf, .value = value });
                    if (oldEntry)
                        entry_free(oldEntry);
                }
                break;  // exit switch(command)
                
              readError:
                fputws(L"hashmap: invalid hash file\n", stderr);
                goto cleanup;
            }
            // unreachable - no break needed
        case HASHFILE_WRITE:
            fwrite(&hashfile_marker, 2, 1, fp);
            fwrite(&(uint32_t){ (uint32_t) hashmap_count(map->hashmap) }, 4, 1, fp);
            hashmap_scan(map->hashmap, entry_iter_write_entry, fp);
            break;
        }
        success = true;
    }
    goto cleanup;

  showHelp:
    _putws(L"Usage: hashfile <handle> < /R | /M | /W > <filename>\n"
           L"\n"
           L"   /R = read hash entries from file, discarding any current entries\n"
           L"   /M = read hash entries from file, merging them with any current entries\n"
           L"   /W = write hash entries to file"
           );
    // fall through to cleanup:
    
  cleanup:
    if (fp && fclose(fp) != 0)
        _wperror(L"hashfile");
    if (argv && LocalFree(argv) != NULL)
        fputws(L"hashmap: LocalFree failed\n", stderr);
    return success ? 0 : -1;
}

/*
 * Frees all outstanding maps.
 */
PLUGIN_API INT WINAPI hashfreeall(LPTSTR argStr) {
    LPWSTR *argv = NULL;
    int numArgs;
    bool verbose = false;
    bool success = false;

    while (iswspace((wint_t) *argStr)) argStr++;  // Trim leading space
    if (*argStr != L'\0') {
        argv = CommandLineToArgvW(argStr, &numArgs);
        if (argv == NULL || numArgs > 1)
            goto showHelp;
        wchar_t *flag = argv[0];
        if (_wcsicmp(flag, L"/V") == 0)
            verbose = true;
        else
            goto showHelp;
    }
    int numFreed = 0;
    for (unsigned int i = 0; i < handleCapacity; i++) {
        availableHandles[i] = i;  // reset the handle indices
        if (mapPtrs[i] != NULL) {
            hashmap_free(mapPtrs[i]->hashmap);
            _set_errno(0);
            free(mapPtrs[i]);
            if (errno != 0)
                _wperror(L"hashmap shutdown");
            mapPtrs[i] = NULL;
            numFreed++;
        }
    }
    if (numFreed && verbose)
        wprintf(L"Freed %d outstanding map(s)\n", numFreed);
    success = true;
    goto cleanup;

  showHelp:
    _putws(L"Usage: hashfreeall [/V]\n"
           L"\n"
           L"   /V = verbose; print number of hashmaps freed"
           );
    // fall through to cleanup

  cleanup:
    if (argv && LocalFree(argv) != NULL)
        fputws(L"hashmap: LocalFree failed\n", stderr);
    return success ? 0 : -1;
}

