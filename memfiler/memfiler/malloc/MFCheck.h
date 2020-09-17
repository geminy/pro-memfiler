/**
 * @file MFCheck.h
 * @brief Memory check
 * @note c/cpp
 */

#ifndef MF_MFCHECK_H
#define MF_MFCHECK_H

#include <stddef.h>
#include <sys/cdefs.h>

/* Arbitrary magical numbers. */
#define MF_MAGICWORD       0xfedabeeb
#define MF_MAGICFREE       0xd8675309
#define MF_MAGICBYTE       ((char)0xd7)
#define MF_MALLOCFLOOD     ((char)0x93)
#define MF_FREEFLOOD       ((char)0x95)

/**
 * @enum MFMCheckStatus
 * @brief Memory check status
 */
enum MFMCheckStatus
{
    MCK_DISABLED = -1,    /* Consistency checking is not turned on. */
    MCK_OK,               /* Block is fine. */
    MCK_FREE,             /* Block freed twice. */
    MCK_HEAD,             /* Memory before the block was clobbered. */
    MCK_TAIL              /* Memory after the block was clobbered. */
};

/**************************************************/

struct MFBacktraceInfo;

/**
 * @struct MFHDR
 * @brief Memory block header
 */
struct MFHDR
{
    size_t size;                 /* Exact size requested by user. */
    unsigned long int magic;     /* Magic number to check header integrity. */
    struct MFHDR *prev;
    struct MFHDR *next;
    __ptr_t block;               /* Real block allocated, for memalign. */
    unsigned long int magic2;    /* Extra, keeps us doubleword aligned. */
    MFBacktraceInfo *bt;         /* Backtrace information from here. */

    MFHDR()
        : size(0)
        , magic(0)
        , prev(NULL)
        , next(NULL)
        , block(NULL)
        , magic2(0)
        , bt(NULL)
    {
    }
};

/**************************************************/

/**
 * @struct MFMallocStatistic
 * @brief Malloc statistic
 */
struct MFMallocStatistic
{
    size_t total;       /* Total size in use. */
    size_t malloced;    /* Times of malloc. */
    size_t freed;       /* Times of free. */

    MFMallocStatistic()
        : total(0)
        , malloced(0)
        , freed(0)
    {
    }
};

/**************************************************/

/**
 * @struct MFRecorder
 * @brief Recorder data
 */
struct MFRecorder
{
    size_t tail;    /* Size of guard. */
    size_t bt;      /* Size of backtrace stack. */

    MFRecorder()
        : tail(1)
        , bt(8)
    {
    }
};

#endif // MF_MFCHECK_H
