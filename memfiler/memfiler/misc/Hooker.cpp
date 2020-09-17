/**
 * malloc hook:
 *     __malloc_hook
 *     __free_hook
 *     __realloc_hook
 *     __memalign_hook
 * work:
 *     malloc/free/calloc/realloc/memalign
 *     new/delete/new[]/delete[]
 *     fopen/fclose ...
 * warn:
 *     Not thread-safe
 */

#include "Global.h"
#include "base/MFBase.h"

MF_BEGIN_DECLS

void*    (*__malloc_hook)(size_t, const void*);
void     (*__free_hook) (void*, const void *);
void*    (*__realloc_hook)(void*, size_t, const void*);
void*    (*__memalign_hook)(size_t , size_t, const void *);

MF_END_DECLS

static void*    (*_old_malloc_hook)(size_t, const void*);
static void     (*_old_free_hook) (void*, const void *);
static void*    (*_old_realloc_hook)(void*, size_t, const void*);
static void*    (*_old_memalign_hook)(size_t , size_t, const void *);

/**************************************************/

static void* _MFMallocHook(size_t size, const void*)
{
    __malloc_hook = _old_malloc_hook;
    void *p = MFMalloc(size);
    __malloc_hook = _MFMallocHook;

    return p;
}

static void _MFFreeHook(void *ptr, const void*)
{
    __free_hook = _old_free_hook;
    MFFree(ptr);
    __free_hook = _MFFreeHook;
}

static void* _MFReallocHook(void *ptr, size_t size, const void*)
{
    __realloc_hook = _old_realloc_hook;
    void *p = MFRealloc(ptr, size);
    __realloc_hook = _MFReallocHook;

    return p;
}

static void* _MFMemalignHook(size_t alignment, size_t size, const void*)
{
    __memalign_hook = _old_memalign_hook;
    void *p = MFMemalign(alignment, size);
    __memalign_hook = _MFMemalignHook;

    return p;
}

/**************************************************/

MF_DECLS
void MFMallocHookInit()
{
    logf();

    _old_malloc_hook = __malloc_hook;
    _old_free_hook = __free_hook;
    _old_realloc_hook = __realloc_hook;
    _old_memalign_hook = __memalign_hook;

    __malloc_hook = _MFMallocHook;
    __free_hook = _MFFreeHook;
    __realloc_hook = _MFReallocHook;
    __memalign_hook = _MFMemalignHook;
}

MF_DECLS
void MFMallocHookDnit()
{
    logf();

    __malloc_hook = _old_malloc_hook;
    __free_hook = _old_free_hook;
    __realloc_hook = _old_realloc_hook;
    __memalign_hook = _old_memalign_hook;
}
