
#ifndef VIS_SCRIPT_HEADER_INCLUDED_
#define VIS_SCRIPT_HEADER_INCLUDED_ 1

struct flist;

typedef struct script_cb {
    struct script* owner;
    char* fn_name;
    char* fn_code;
} *script_cb_t;

typedef struct script* script_t;

script_t script_new(void);
void script_destroy(script_t s);
struct flist* script_run(script_t script, const char* filename);
void call_script(script_t state, script_cb_t func, void* args);

#endif

