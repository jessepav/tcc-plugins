#include "pch.h"

#include "PlugIn.h"
#include "TakeCmd.h"

#include "swizzle.h"

PLUGIN_API INT WINAPI f_swizzle(LPTSTR lpszString) {
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
