/**
 * @file Global.h
 * @brief Global definitions
 * @note c/cpp
 */

#ifndef MF_GLOBAL_H
#define MF_GLOBAL_H

#include <stdio.h>

/* log */
#define MF_USE_DEBUG
#ifdef MF_USE_DEBUG
#define logn() \
    do { \
        printf("\n"); \
        fflush(stdout); \
    } while (0)
# define logf() \
    do { \
        printf("[%s]\n", __PRETTY_FUNCTION__); \
        fflush(stdout); \
    } while (0)
# define logl() \
    do { \
        printf("[%s] [%d]\n", __FILE__, __LINE__); \
        fflush(stdout); \
    } while (0)
# define loga() \
    do { \
        printf("[%s] [%s] [%d]\n", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
        fflush(stdout); \
    } while (0)
# define logp(format, ...) \
        do { \
        printf(format, ##__VA_ARGS__); \
        fflush(stdout); \
    } while (0)
#else
#define logn()
# define logf()
# define logl()
# define loga()
# define logp(format, ...)
#endif

/* platform */
#if defined(linux) || defined (__linux) || defined(__GNUC__)
# define MF_CONSTRUCTOR __attribute__((constructor))
# define MF_DESTRUCTOR __attribute__((destructor))
#else
# define MF_CONSTRUCTOR
# define MF_DESTRUCTOR
#endif

/* extern */
#ifdef	__cplusplus
# define MF_BEGIN_DECLS extern "C" {
# define MF_END_DECLS }
# define MF_DECLS extern "C"
#else
# define MF_BEGIN_DECLS
# define MF_END_DECLS
# define MF_DECLS
#endif

/* fifo: mf_pid.fifo */
#define MF_FIFO_PATHSIZE (128)
#define MF_FIFO_CMDSIZE (16)
#define MF_FIFO_PREFIX "mf_"
#define MF_FIFO_SUFFIX ".fifo"
#define MF_FIFO_MODE (0777)
#define MF_FIFO_DUMP "dump"
#define MF_FIFO_CHECK "check"
#define MF_FIFO_QUIT "quit"

#endif // MF_GLOBAL_H
