/**
 * @file TestSuite.h
 * @brief Test suite
 */

#ifndef TESTSUITE_H
#define TESTSUITE_H

#include <iostream>
#include <string>

/**
 * @class TestSuite
 * @brief Test suite
 */
class TestSuite
{
public:
    explicit TestSuite(const std::string &module)
        : m_module(module)
    {
    }

    virtual ~TestSuite() {}

    virtual void run() = 0;

    void begin()
    {
        std::cout << "********** ";
        std::cout << m_module;
        std::cout << " BEGIN";
        std::cout << " **********";
        std::cout << std::endl;
    }

    void end()
    {
        std::cout << "********** ";
        std::cout << m_module;
        std::cout << " END";
        std::cout << " **********";
        std::cout << std::endl;
    }

private:
    std::string m_module;
};

/**************************************************/

#define TEST_RUN(Class) \
    int main(int argc, char **argv) \
    { \
        static_cast<void>(argc); \
        static_cast<void>(argv); \
        std::cout << "********** TEST BEGIN **********" << std::endl; \
        std::cout << std::endl; \
        Class _test; \
        _test.begin(); \
        _test.run(); \
        _test.end(); \
        std::cout << std::endl; \
        std::cout << "********** TEST END **********" << std::endl; \
        return 0; \
    }

#endif // TESTSUITE_H
