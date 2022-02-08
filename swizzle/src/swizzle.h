#define PLUGIN_API __declspec(dllexport)

#define STRBUF_SIZE 4096
#define ARGBUF_SIZE 1024
#define MAX_FUNCSTR_SIZE 32000   // leave a few hundred charcters to spare
#define MAX_PARAMS 20

extern WCHAR *strbuf;
extern WCHAR *argbuf;
