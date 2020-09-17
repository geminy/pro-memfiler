#include "Global.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#define TestMallocFile "testfile"

void test_malloc_c()
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
    p = (char*)memalign(8, 40);
    memset(p, '3', 35);
    logp("memalign: addr=%p size=%lu str=%s\n", p, strlen(p), p);
    free(p);
    logp("free: addr=%p size=%lu str=%s\n", p, strlen(p), p);

    FILE *f = fopen(TestMallocFile, "w");
    if (NULL != f) {
        logp("fopen: addr=%p file=%s mode=%s\n", f, TestMallocFile, "w");
        fclose(f);
        logp("fclose: addr=%p file=%s mode=%s\n", f, TestMallocFile, "w");
    }
}
