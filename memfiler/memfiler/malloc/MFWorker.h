/**
 * @file MFWorker.h
 * @brief Memory profiler worker
 */

#ifndef MF_MFWORKER_H
#define MF_MFWORKER_H

#ifndef __cplusplus
# error ERROR: Must be included at .cpp file
#endif

#include "memfiler/base/MFMutex.h"
#include "memfiler/malloc/MFCheck.h"

struct MFHDR;

/**
 * @class MFMallocWorker
 * @brief Malloc worker
 */
class MFMallocWorker
{
public:
    MFMallocWorker();
    ~MFMallocWorker();

    void* doMalloc(size_t size);
    void doFree(void *ptr);
    void* doCalloc(size_t nmemb, size_t size);
    void* doRealloc(void *ptr, size_t size);
    void* doMemalign(size_t alignment, size_t size);
    int doMallopt(int param, int value);

    int checkAllBlock();
    void recordBacktrace();
    void dumpAll();
    void dumpInfo();
    void dumpMaps();
    void dumpSmaps();

private:
    bool isValidSize(size_t size);
    size_t extendSize(size_t size);

    void linkBlock(MFHDR *hdr);
    void unlinkBlock(MFHDR *ptr);
    MFMCheckStatus checkBlock(MFHDR *ptr);
    void badBlock(MFHDR *ptr, MFMCheckStatus stat);

    MFMutex m_mutex;
    MFMallocStatistic m_statistic;
    MFRecorder m_recorder;
    bool m_pedantic;
    char *m_tailRef;
    MFHDR *m_root;
};

#endif // MF_MFWORKER_H
