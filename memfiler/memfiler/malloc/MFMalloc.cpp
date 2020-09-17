/**
 * internal malloc:
 *     MFMalloc
 *     MFFree
 *     MFCalloc
 *     MFRealloc
 *     MFMemalign
 *     MFMallopt
 * external malloc:
 *     __libc_malloc
 *     __libc_free
 *     __libc_calloc
 *     __libc_realloc
 *     __libc_memalign
 *     __libc_mallopt
 */

#include "Global.h"
#include "base/MFBase.h"
#include "Memfiler.h"

MF_DECLS
void* MFMalloc(size_t size)
{
    return _mf->mallocWorker()->doMalloc(size);
}

MF_DECLS
void MFFree(void *ptr)
{
    _mf->mallocWorker()->doFree(ptr);
}

MF_DECLS
void* MFCalloc(size_t nmemb, size_t size)
{
    return _mf->mallocWorker()->doCalloc(nmemb, size);
}

MF_DECLS
void* MFRealloc(void *ptr, size_t size)
{
    return _mf->mallocWorker()->doRealloc(ptr,size);
}

MF_DECLS
void* MFMemalign(size_t alignment, size_t size)
{
    return _mf->mallocWorker()->doMemalign(alignment, size);
}

MF_DECLS
int MFMallopt(int param, int value)
{
    return _mf->mallocWorker()->doMallopt(param, value);
}

/**************************************************/

#ifdef MF_HOOK_C

MF_DECLS
void* malloc(size_t size)
{
    return MFMalloc(size);
}

MF_DECLS
void free(void *ptr)
{
    MFFree(ptr);
}

MF_DECLS
void* calloc(size_t nmemb, size_t size)
{
    return MFCalloc(nmemb, size);
}

MF_DECLS
void* realloc(void *ptr, size_t size)
{
    return MFRealloc(ptr, size);
}

MF_DECLS
void* memalign(size_t alignment, size_t size)
{
    return MFMemalign(alignment, size);
}

MF_DECLS
void* __libc_memalign(size_t alignment, size_t size)
{
    return MFMemalign(alignment, size);
}

MF_DECLS
int mallopt(int param, int value)
{
    return MFMallopt(param, value);
}

#endif // MF_HOOK_C

/**************************************************/

#ifdef MF_HOOK_CXX

#include <new>

void* operator new(std::size_t size) throw (std::bad_alloc)
{
    logf();

    if (size < 1) {
        return NULL;
    }
    void *p = MFMalloc(size);
    if (NULL == p) {
        throw std::bad_alloc();
    }
    return p;
}

void* operator new[](std::size_t size) throw (std::bad_alloc)
{
    logf();

    if (size < 1) {
        return NULL;
    }
    void *p = MFMalloc(size);
    if (NULL == p) {
        throw std::bad_alloc();
    }
    return p;
}

void operator delete(void *ptr) throw()
{
    logf();

    if (NULL != ptr) {
        MFFree(ptr);
    }
}

void operator delete[](void *ptr) throw()
{
    logf();

    if (NULL != ptr) {
        MFFree(ptr);
    }
}

void* operator new(std::size_t size, const std::nothrow_t&) throw()
{
    logf();

    if (size < 1) {
        return NULL;
    }
    void *p = NULL;
    do {
        p = MFMalloc(size);
    } while (NULL == p);
    return p;
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw()
{
    logf();

    if (size < 1) {
        return NULL;
    }
    void *p = NULL;
    do {
        p = MFMalloc(size);
    } while (NULL == p);
    return p;
}

void operator delete(void *ptr, const std::nothrow_t&) throw()
{
    logf();

    if (NULL != ptr) {
        MFFree(ptr);
    }
}

void operator delete[](void *ptr, const std::nothrow_t&) throw()
{
    logf();

    if (NULL != ptr) {
        MFFree(ptr);
    }
}

/*
// Default placement versions of operator new and delete.
inline void* operator new(size_t, void* __p) throw() { return __p; }
inline void* operator new[](std::size_t, void* __p) throw() { return __p; }
inline void operator delete(void*, void*) throw() {}
inline void operator delete[](void*, void*) throw() {}
*/

#endif // MF_HOOK_CXX
