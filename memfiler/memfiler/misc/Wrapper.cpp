/**
 * gcc wrap:
 *     malloc
 *     free
 *     calloc
 *     realloc
 *     memalign
 * work:
 *     malloc/free/calloc/realloc/memalign
 *     new/delete/new[]/delete[]
 *     fopen/fclose ...
 */

#include "Global.h"
#include "base/MFBase.h"

#ifdef MF_USE_WRAP

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

#endif // MF_USE_WRAP
