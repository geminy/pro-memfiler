#include "Global.h"
#include "base/MFBase.h"
#include "backtrace/MFBacktrace.h"
#include "MFWorker.h"

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <ctype.h>
#include <unistd.h>

#define MF_PATHSIZE MF_FIFO_PATHSIZE

extern void*(*__mf__libc_memalign)(size_t, size_t);

/**************************************************/

static void _MFProbe(FILE *file, unsigned char *addr, int from, int size)
{
    char _tmp = ' ';
    for (int i = from; i < from + size; i += 8) {
        fprintf(file, "[ %p ] [ ", addr + i);
        for (int j = 0; j < 8; ++j) {
            fprintf(file, "%02x ", *(addr + i + j));
        }
        fprintf(file, "] [ ");
        for (int j = 0; j < 8; ++j) {
            _tmp = *(addr + i + j);
            fprintf(file, "%c", (isprint(_tmp) ? _tmp : '.'));
        }
        fprintf(file, " ]\n");
    }
}

/**************************************************/

MFMallocWorker::MFMallocWorker()
    : m_pedantic(false)
    , m_tailRef(NULL)
    , m_root(NULL)
{
    m_tailRef = (char*)__libc_malloc(m_recorder.tail);
    assert(NULL != m_tailRef);
    memset(m_tailRef, MF_MAGICBYTE, m_recorder.tail);
}

MFMallocWorker::~MFMallocWorker()
{
}

void* MFMallocWorker::doMalloc(size_t size)
{
    if (m_pedantic) {
        checkAllBlock();
    }

    // underflow
    if (size < 1) {
        logp("MFMallocWorker::doMalloc failed for INVALID ARG size=%zu\n", size);
        return NULL;
    }

    // overflow
    if (!isValidSize(size)) {
        logp("MFMallocWorker::doMalloc failed for ENOMEM size=%zu\n", size);
        return NULL;
    }

    MFHDR *hdr = (MFHDR*)__libc_malloc(extendSize(size));
    if (NULL == hdr) {
        logp("MFMallocWorker::doMalloc failed for __libc_malloc extend=%zu\n", extendSize(size));
        return NULL;
    }

    MFRecordBacktrace(hdr, m_recorder.bt);
    hdr->size = size;
    hdr->block = hdr;
    hdr->magic2 = (uintptr_t)hdr ^ MF_MAGICWORD;
    memset(((char*)&hdr[1]) + size, MF_MAGICBYTE, m_recorder.tail);
    memset((__ptr_t)(hdr + 1), MF_MALLOCFLOOD, size);

    m_mutex.lock();
    linkBlock(hdr);
    m_mutex.unlock();

    return (__ptr_t)(hdr + 1);
}

void MFMallocWorker::doFree(void *ptr)
{
    if (m_pedantic) {
        checkAllBlock();
    }

    // NULL
    if (NULL == ptr) {
        logp("MFMallocWorker::doFree failed for INVALID ARG NULL\n");
        return;
    }

    MFHDR *hdr = ((MFHDR*)ptr) - 1;

    m_mutex.lock();
    MFMCheckStatus stat = checkBlock(hdr);
    if (MCK_OK != stat) {
        logp("MFMallocWorker::doFree failed for BAD BLOCK addr=%p status=%d\n", ptr, stat);
        unlinkBlock(hdr);
        m_mutex.unlock();
        badBlock(hdr, stat);
        abort();
    }
    else {
        unlinkBlock(hdr);
        m_mutex.unlock();
    }

    hdr->magic = MF_MAGICFREE;
    hdr->magic2 = MF_MAGICFREE;
    hdr->prev = hdr->next = NULL;
    if (NULL != hdr->bt) {
        __libc_free(hdr->bt);
        hdr->bt = NULL;
    }

    memset(ptr, MF_FREEFLOOD, hdr->size);
    ptr = hdr->block;
    __libc_free(ptr);
}

void* MFMallocWorker::doCalloc(size_t nmemb, size_t size)
{
    // underflow & overflow
    if (nmemb < 1 || size < 1 || ~((size_t)0) / nmemb < size) {
        logp("MFMallocWorker::doCalloc failed for INVALID ARG nmemb=%zu size=%zu\n", nmemb, size);
        return NULL;
    }

    size_t sz = nmemb * size;
    void *p = doMalloc(sz);
    if (NULL != p) {
        memset(p, 0 ,size);
    }
    return p;
}

void* MFMallocWorker::doRealloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        logp("MFMallocWorker::doRealloc do malloc for NULL PTR\n");
        ptr = doMalloc(size);
        return ptr;
    }

    if (0 == size) {
        logp("MFMallocWorker::doRealloc do free for ZERO SIZE\n");
        doFree(ptr);
        return NULL;
    }

    if (m_pedantic) {
        checkAllBlock();
    }

    // overflow
    if (!isValidSize(size)) {
        logp("MFMallocWorker::doRealloc failed for ENOMEM size=%zu\n", size);
        return NULL;
    }

    MFHDR *hdr;
    size_t osize;

    if (NULL != ptr) {
        hdr = ((MFHDR*)ptr) - 1;
        osize = hdr->size;

        m_mutex.lock();
        MFMCheckStatus stat = checkBlock(hdr);
        if (MCK_OK != stat) {
            logp("MFMallocWorker::doRealloc failed for BAD BLOCK addr=%p status=%d\n", ptr, stat);
            unlinkBlock(hdr);
            m_mutex.unlock();
            badBlock(hdr, stat);
            abort();
        }
        else {
            unlinkBlock(hdr);
            m_mutex.unlock();

            hdr->magic = MF_MAGICFREE;
            hdr->magic2 = MF_MAGICFREE;
            hdr->prev = hdr->next = NULL;
            if (NULL != hdr->bt) {
                __libc_free(hdr->bt);
                hdr->bt = NULL;
            }

            if (size < osize) {
                memset((char*)ptr + size, MF_FREEFLOOD, osize - size);
            }
        }
    }
    else {
        hdr = NULL;
        osize = 0;
    }

    hdr = (MFHDR*)__libc_realloc((__ptr_t)hdr, extendSize(size));
    if (NULL == hdr) {
        logp("MFMallocWorker::doRealloc failed for __libc_realloc extend=%zu\n", extendSize(size));
        return NULL;
    }

    MFRecordBacktrace(hdr, m_recorder.bt);
    hdr->size = size;
    hdr->block = hdr;
    hdr->magic2 = (uintptr_t)hdr ^ MF_MAGICWORD;
    memset(((char*)&hdr[1]) + size, MF_MAGICBYTE, m_recorder.tail);
    if (size > osize) {
        memset((char*)(hdr + 1) +osize, MF_MALLOCFLOOD, size - osize);
    }

    m_mutex.lock();
    linkBlock(hdr);
    m_mutex.unlock();

    return (__ptr_t)(hdr + 1);
}

void* MFMallocWorker::doMemalign(size_t alignment, size_t size)
{
    if (m_pedantic) {
        checkAllBlock();
    }

    // ensure alignment is power of 2, not larger than original alignment
    if (alignment & (alignment - 1)) {
        alignment = 1L << (31 - __builtin_clz(alignment));
    }

    // slop is several times of alignment
    MFHDR *hdr = NULL;
    size_t slop = (sizeof *hdr + alignment -1) & -alignment;

    // oveflow
    if (size > ~((size_t)0)  - (slop + m_recorder.tail)) {
        logp("MFMallocWorker::doMemalign failed for ENOMEM alignment=%zu size=%zu\n", alignment, size);
        return NULL;
    }

//    char *block = (char*)__libc_memalign(alignment, slop + size + m_recorder.tail);
    char *block = (char*)__mf__libc_memalign(alignment, slop + size + m_recorder.tail);
    if (NULL == block) {
        logp("MFMallocWorker::doMemalign failed for __libc_memalign alignment=%zu size=%zu\n", alignment, size);
        return NULL;
    }

    hdr = (MFHDR*)(block + slop) - 1;
    MFRecordBacktrace(hdr, m_recorder.bt);
    hdr->size = size;
    hdr->block = hdr;
    hdr->magic2 = (uintptr_t)hdr ^ MF_MAGICWORD;
    memset(((char*)&hdr[1]) + size, MF_MAGICBYTE, m_recorder.tail);
    memset((__ptr_t)(hdr + 1), MF_MALLOCFLOOD, size);

    m_mutex.lock();
    linkBlock(hdr);
    m_mutex.unlock();

    return (__ptr_t)(hdr + 1);
}

int MFMallocWorker::doMallopt(int param, int value)
{
    return __libc_mallopt(param, value);
}

int MFMallocWorker::checkAllBlock()
{
    /* Walk through all the active blocks and test whether they were tampered with. */
    MFAutoMutex am(m_mutex);

    int errors = 0;
    MFHDR *runp = m_root;
    while (NULL != runp) {
        MFMCheckStatus stat = checkBlock(runp);
        if (MCK_OK != stat) {
            ++errors;
            badBlock(runp, stat);
        }
        runp = runp->next;
    }

    return errors;
}

void MFMallocWorker::recordBacktrace()
{
    FILE *bt = fopen("mf_backtrace.log", "w");
    if (NULL != bt) {
        size_t _stack = 0;
        intptr_t _bt[MF_BACKTRACE_SIZE] = { 0 };
        _stack = MFGetBacktrace(_bt, 8);
        fprintf(bt, "BACKTRACE\n");
        fprintf(bt, "%zu\n", _stack);
        for (size_t i = 0; i < _stack; ++i) {
            if (0 == _bt[i]) {
                break;
            }
            else {
                fprintf(bt, "%08x ", (unsigned int)_bt[i]);
            }
        }
        fprintf(bt, "\n");
        fprintf(bt, "END\n");
//        fclose(bt);
    }
}

void MFMallocWorker::dumpAll()
{
    // without fclose
    dumpInfo();
    dumpMaps();
    dumpSmaps();
}

void MFMallocWorker::dumpInfo()
{
    FILE *info = fopen("mf_info.log", "w");
    if (NULL != info) {
        fprintf(info, "INFO\n");
        fprintf(info, "bt=%zu tail=%zu\n", m_recorder.bt, m_recorder.tail);
        m_mutex.lock();
        fprintf(info, "total=%zu malloc=%zu free=%zu\n", m_statistic.total, m_statistic.malloced, m_statistic.freed);
        m_mutex.unlock();
        fprintf(info, "END\n");
//        fclose(info);
    }
}

void MFMallocWorker::dumpMaps()
{
    FILE *maps = fopen("/proc/self/maps", "r");
    if (NULL != maps) {
        FILE *log = fopen("mf_maps.log", "w");
        if (NULL != log) {
            char buf[BUFSIZ] = { 0 };
            size_t n = 0;
            fprintf(log, "MAPS\n");
            while ((n = fread(buf, sizeof(char), BUFSIZ, maps)) > 0) {
                fwrite(buf, sizeof(char), n, log);
                memset(buf, 0, BUFSIZ);
            }
            fprintf(log, "END\n");
//            fclose(log);
        }
        fclose(maps);
    }
}

void MFMallocWorker::dumpSmaps()
{
    char path[MF_PATHSIZE] = { 0 };
    sprintf(path, "/proc/%d/smaps", getpid());
    FILE *smaps = fopen(path, "r");
    if (NULL != smaps) {
        FILE *log = fopen("mf_smaps.log", "w");
        if (NULL != log) {
            char buf[BUFSIZ] = { 0 };
            size_t n = 0;
            fprintf(log, "SMAPS\n");
            while ((n = fread(buf, sizeof(char), BUFSIZ, smaps)) > 0) {
                fwrite(buf, sizeof(char), n, log);
                memset(buf, 0, BUFSIZ);
            }
            fprintf(log, "END\n");
//            fclose(log);
        }
        fclose(smaps);
    }
}

bool MFMallocWorker::isValidSize(size_t size)
{
    if (size > ~((size_t)0) - (sizeof(MFHDR) + m_recorder.tail)) {
        return false;
    }
    return true;
}

size_t MFMallocWorker::extendSize(size_t size)
{
    return (sizeof(MFHDR) + size + m_recorder.tail);
}

void MFMallocWorker::linkBlock(MFHDR *hdr)
{
    hdr->prev = NULL;
    hdr->next = m_root;
    m_root = hdr;
    // magic = MF_MAGICWORD ^ next
    hdr->magic = MF_MAGICWORD ^ (uintptr_t)hdr->next;

    /* And the next block. */
    if (NULL != hdr->next) {
        hdr->next->prev = hdr;
        // magic = MF_MAGICWORD ^ (prev + next)
        hdr->next->magic = MF_MAGICWORD ^ ((uintptr_t)hdr + (uintptr_t)hdr->next->next);
    }

    m_statistic.total += hdr->size;
    m_statistic.malloced += 1;
}

void MFMallocWorker::unlinkBlock(MFHDR *ptr)
{
    if (NULL != ptr->next)
    {
        ptr->next->prev = ptr->prev;
        ptr->next->magic = MF_MAGICWORD ^ ((uintptr_t)ptr->next->prev + (uintptr_t)ptr->next->next);
    }
    if (NULL != ptr->prev)
    {
        ptr->prev->next = ptr->next;
        ptr->prev->magic = MF_MAGICWORD ^ ((uintptr_t)ptr->prev->prev + (uintptr_t)ptr->prev->next);
    }
    else {
        m_root = ptr->next;
    }

    m_statistic.total -= ptr->size;
    m_statistic.freed += 1;
}


MFMCheckStatus MFMallocWorker::checkBlock(MFHDR *ptr)
{
    MFMCheckStatus status;

    switch (ptr->magic ^ ((uintptr_t)ptr->prev + (uintptr_t)ptr->next)) {
    default:
        status = MCK_HEAD;
        break;
    case MF_MAGICFREE:
        status = MCK_FREE;
        break;
    case MF_MAGICWORD:
        if (memcmp(((char*)&ptr[1]) + ptr->size, m_tailRef, m_recorder.tail) != 0)  {
            status = MCK_TAIL;
        }
        else if ((ptr->magic2 ^ (uintptr_t)ptr->block) != MF_MAGICWORD) {
            status = MCK_HEAD;
        }
        else {
            status = MCK_OK;
        }
        break;
    }

//    if (MCK_OK != status) {
//        badBlock(ptr, status);
//        abort();
//    }

    return status;
}

void MFMallocWorker::badBlock(MFHDR *ptr, MFMCheckStatus stat)
{
    FILE *bad = fopen("mf_bad.log", "w");
    if (NULL != bad) {
        fprintf(bad, "BAD\n");
        fprintf(bad, "addr=%p stat=%d size=%zu\n", ptr, stat, ptr->size);
        if (NULL != ptr->bt) {
            for (size_t i = 0; i < ptr->bt->stack; ++i) {
                if (0 == ptr->bt->bt[i]) {
                    break;
                }
                else {
                    fprintf(bad, "%08x ", (unsigned int)ptr->bt->bt[i]);
                }
            }
        }
        fprintf(bad, "\n");

        if (MCK_TAIL == stat) {
            fprintf(bad, "BEFORE\n");
            _MFProbe(bad, (unsigned char*)ptr, -128, 128);
            fprintf(bad, "HDR\n");
            _MFProbe(bad, (unsigned char*)ptr, 0, sizeof(MFHDR));
            fprintf(bad, "USER\n");
            _MFProbe(bad, (unsigned char*)ptr, sizeof(MFHDR), ptr->size);
            fprintf(bad, "TAIL\n");
            _MFProbe(bad, (unsigned char*)ptr, sizeof(MFHDR) + ptr->size, m_recorder.tail);
            fprintf(bad, "AFTER\n");
            _MFProbe(bad, (unsigned char*)ptr, sizeof(MFHDR) + ptr->size + m_recorder.tail, 128);
        }

        fprintf(bad, "END\n");
//        fclose(bad);
    }
}
