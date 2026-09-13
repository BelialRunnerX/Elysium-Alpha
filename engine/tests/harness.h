// The test harness: a counter, a macro and a section header.
//
// Deliberately about thirty lines. A test framework is a dependency, and this
// project has one dependency (zlib) for a reason — the value of these tests is
// in what they assert, not in how they are run. CHECK reports the file, the
// line and a formatted message, which is everything a failure needs to be
// actionable.
#pragma once
#include <cstdio>

namespace elytest {

extern int gFail;
extern int gRun;

void section(const char* name);
int report(const char* suite);

}  // namespace elytest

// A check with no message, for the inside of a loop that runs thousands of
// times. It still counts, and it still reports the file and line on failure —
// it simply has nothing useful to add beyond "this was false here".
#define CHECK_QUIET(cond)                                                   \
    do {                                                                    \
        ++elytest::gRun;                                                    \
        if (!(cond)) { ++elytest::gFail;                                    \
            std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); } \
    } while (0)

#define CHECK(cond, ...)                                                    \
    do {                                                                    \
        ++elytest::gRun;                                                    \
        if (!(cond)) { ++elytest::gFail;                                    \
            std::printf("  FAIL %s:%d  ", __FILE__, __LINE__);              \
            std::printf(__VA_ARGS__); std::printf("\n"); }                  \
    } while (0)
