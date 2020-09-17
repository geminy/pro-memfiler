/**
 * @file MFBacktrace.h
 * @brief Backtrace definitions
 * @note c/cpp
 */

#ifndef MF_MFBACKTRACE_H
#define MF_MFBACKTRACE_H

#include "Global.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @struct MFBacktraceInfo
 * @brief Backtrace information
 */
struct MFBacktraceInfo
{
    size_t stack;      /* Size of backtrace stacks. */
    intptr_t bt[0];    /* Stores backtrace addresses. */
};

/**************************************************/

MF_DECLS int MFGetBacktrace(intptr_t *addr, size_t stack);

/**************************************************/

#define MF_BACKTRACE_SIZE (32)

#define MFRecordBacktrace(HDR, STACK) \
    do { \
        if (NULL != hdr) { \
            size_t _stack = 0; \
            intptr_t _bt[MF_BACKTRACE_SIZE] = { 0 }; \
            _stack = MFGetBacktrace(_bt, STACK); \
            if (NULL != hdr->bt) { \
                __libc_free(hdr->bt); \
            } \
            hdr->bt = (struct MFBacktraceInfo*)__libc_malloc(sizeof(struct MFBacktraceInfo) + _stack * sizeof(intptr_t)); \
            if (NULL != hdr->bt) { \
                hdr->bt->stack = _stack; \
                memcpy(hdr->bt->bt, _bt, _stack * sizeof(intptr_t)); \
            } \
        } \
    } while (0)

#endif // MF_MFBACKTRACE_H
