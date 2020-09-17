#include "Global.h"
#include "Memfiler.h"
#include "memfiler/backtrace/MFBacktrace.h"
#include "memfiler/base/MFSystemHandle.h"
#include "memfiler/malloc/MFWorker.h"
#include "TestMemfiler.h"

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <malloc.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#define TestMemfilerTag "TestMemfiler"
#define TestMemfilerFile "testfile"

MF_DECLS void test_malloc_c();

/**************************************************/

/**
 * @class _Object
 * @brief For test
 * @note sizeof(_Object)=16 virtual=8 int=4 alignment=8
 */
class _Object
{
public:
    _Object() {}
    virtual ~_Object() {}

private:
    int m_count;
};

/**************************************************/

TestMemfiler::TestMemfiler()
    : TestSuite(TestMemfilerTag)
{
}

TestMemfiler::~TestMemfiler()
{
}

void TestMemfiler::run()
{
    test_bt();
    test_handle();
    test_worker();
    test_memfiler();
    test_malloc();
    test_ipc();
}

void TestMemfiler::test_bt()
{
    logn();
    loga();

    size_t _stack = 0;
    intptr_t _bt[MF_BACKTRACE_SIZE] = { 0 };
    _stack = MFGetBacktrace(_bt, 8);
    for (size_t i = 0; i < _stack; ++i) {
        if (0 == _bt[i]) {
            break;
        }
        else {
            logp("%08x ", (unsigned int)_bt[i]);
        }
    }
    logn();
}

void TestMemfiler::test_handle()
{
    logn();
    loga();

    MFSystemHandle().handle();
}

void TestMemfiler::test_worker()
{
    logn();
    loga();

    _mf->mallocWorker()->checkAllBlock();
    _mf->mallocWorker()->dumpAll();
    _mf->mallocWorker()->recordBacktrace();
}

void TestMemfiler::test_memfiler()
{
    logn();
    loga();

    _mf->InitializeMF();
}

void TestMemfiler::test_malloc()
{
    logn();
    loga();

    char *p = (char*)malloc(10);
    memset(p, '0', 5);
    logp("malloc: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    p = (char*)realloc(p, 20);
    memset(p, '1', 15);
    logp("realloc: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    free(p);
    logp("free: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    p = (char*)calloc(30, 1);
    memset(p, '2', 25);
    logp("calloc: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    free(p);
    logp("free: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    p = (char*)memalign(8, 40); // can not larger than 15
    memset(p, '3', 35);
    logp("memalign: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    free(p);
    logp("free: addr=%p size=%lu str=%s\n", p, strlen(p), p);

    _Object *o = new _Object;
    logp("new: addr=%p size=%lu ptr=%lu\n", o, sizeof(_Object), sizeof(o));
    delete o;
    logp("new: addr=%p size=%lu ptr=%lu\n", o, sizeof(_Object), sizeof(o));
    int *p2 = new int[50];
    for (int i = 0; i < 50; ++i) {
        *(p2 + i) = i + 100;
    }
    logp("new[]: addr=%p first=%d last=%d\n", p2, *p2, *(p2 + 49));
    delete[] p2;
    logp("delete[]: addr=%p first=%d last=%d\n", p2, *p2, *(p2 + 49));

    FILE *f = fopen(TestMemfilerFile, "w");
    if (NULL != f) {
        logp("fopen: addr=%p file=%s mode=%s\n", f, TestMemfilerFile, "w");
        fclose(f);
        logp("fclose: addr=%p file=%s mode=%s\n", f, TestMemfilerFile, "w");
    }

    test_malloc_c();
}

void TestMemfiler::test_ipc()
{
    logn();
    loga();

    char path[MF_FIFO_PATHSIZE] = { 0 };
    sprintf(path, "%s%d%s", MF_FIFO_PREFIX, getpid(), MF_FIFO_SUFFIX);

    int fd = open(path, O_WRONLY);
    if (-1 == fd) {
        logp("@TestMemfiler::test_ipc open failed %s\n", strerror(errno));
        abort();
    }

    const char *test = "test";
    int len = write(fd, test, strlen(test));
    logp("#TestMemfiler::test_ipc write %s %d\n", test, len);
    sleep(1);

    const char *check = "check\n";
    len = write(fd, check, strlen(check));
    logp("#TestMemfiler::test_ipc write %s %d\n", check, len);
    sleep(1);

    const char *dump = "dump";
    len = write(fd, dump, strlen(dump));
    logp("#TestMemfiler::test_ipc write %s %d\n", dump, len);
    sleep(1);

    const char *quit = "quit\n";
    len = write(fd, quit, strlen(quit));
    logp("#TestMemfiler::test_ipc write %s %d\n", quit, len);
    sleep(1);

    close(fd);
}

/**************************************************/

TEST_RUN(TestMemfiler)
