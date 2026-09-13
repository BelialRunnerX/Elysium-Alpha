#include "harness.h"

namespace elytest {

int gFail = 0;
int gRun = 0;

void section(const char* name) { std::printf("\n== %s\n", name); }

int report(const char* suite) {
    std::printf("\n%s: %d checks, %d failures\n", suite, gRun, gFail);
    return gFail == 0 ? 0 : 1;
}

}  // namespace elytest
