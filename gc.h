
#ifndef VIS_GC_HEADER_INCLUDED_
#define VIS_GC_HEADER_INCLUDED_ 1

/** Garbage collection service
 *
 * Call gc_init(), then call gc_add(func, cls) which invokes
 *  (func)(cls)
 * once the program is terminating. Useful for not leaking memory.
 *
 * Example: free an allocation on exit
 *  void* x = malloc(100);
 *  gc_add(free, x);
 */

typedef void (*gc_func_t)(void* cls);

void gc_init(void);

void gc_add(gc_func_t func, void* cls);

#endif
