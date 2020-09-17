/**
 * @file Memfiler.h
 * @brief Memory profiler
 * @note c/cpp
 * @warning Include this file directly instead of <memfiler/Memfiler.h>
 */

#ifdef __cplusplus
# include "memfiler/Memfiler.h"
#endif

#include "Global.h"

MF_BEGIN_DECLS

void*    MFMalloc(size_t size);
void     MFFree(void *ptr);
void*    MFCalloc(size_t nmemb, size_t size);
void*    MFRealloc(void *ptr, size_t size);
void*    MFMemalign(size_t alignment, size_t size);

MF_END_DECLS
