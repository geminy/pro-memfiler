/**
 * @file MFBase.h
 * @brief Base declarations
 * @note c/cpp
 */

#ifndef MF_MFBASE_H
#define MF_MFBASE_H

#include "Global.h"

MF_BEGIN_DECLS

void*    MFMalloc(size_t);
void     MFFree(void*);
void*    MFCalloc(size_t, size_t);
void*    MFRealloc(void*, size_t);
void*    MFMemalign(size_t, size_t);
int      MFMallopt(int, int);

void*    __libc_malloc(size_t);
void     __libc_free(void*);
void*    __libc_calloc(size_t, size_t);
void*    __libc_realloc(void*, size_t);
void*    __libc_memalign(size_t, size_t);
int      __libc_mallopt(int ,int);

MF_END_DECLS

#endif // MF_MFBASE_H
