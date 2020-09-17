/**
 * @file Memfiler.h
 * @brief Memory profiler
 */

#ifndef MF_MEMFILER_H
#define MF_MEMFILER_H

#ifndef __cplusplus
# error ERROR: Must be included at .cpp file
#endif

#include <memfiler/malloc/MFWorker.h>
#include <memfiler/base/MFSystemHandle.h>

/**
 * @class Memfiler
 * @brief Memory profiler
 */
class Memfiler
{
public:
    ~Memfiler();

    MFMallocWorker* mallocWorker();

    static Memfiler* MF();
    static bool InitializeMF();

private:
    Memfiler();

    void initialize();

    static void* MFThreadRoutine(void *arg);
    static bool MFCmdProc(char *cmd, int len);

    MFMallocWorker m_malloc;
    MFSystemHandle m_handle;

    static Memfiler *m_inst;
    static bool m_initialized;
};

/**************************************************/

#define _mf Memfiler::MF()

#endif // MF_MEMFILER_H
