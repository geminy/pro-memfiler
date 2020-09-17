/**
 * @file TestMemfiler.h
 * @brief Memfiler test
 */

#ifndef TESTMEMFILER_H
#define TESTMEMFILER_H

#include "TestSuite.h"

/**
 * @class TestMemfiler
 * @brief Memfiler test
 */
class TestMemfiler : public TestSuite
{
public:
    TestMemfiler();
    ~TestMemfiler();

    void run();

private:
    void test_bt();
    void test_handle();
    void test_worker();
    void test_memfiler();
    void test_malloc();
    void test_ipc();
};

#endif // TESTMEMFILER_H
