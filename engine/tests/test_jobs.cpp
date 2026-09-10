// Job system tests.
//
// Threading bugs do not fail; they fail *sometimes*, on a machine you do not
// have, under a load you cannot reproduce. So the tests here are heavy on
// repetition and on properties that must hold rather than on single runs, and
// the most important one is the last: the same work must produce the same
// answer whether it runs on one thread or on sixteen. That is principle S2, and
// threading is the easiest way in the whole engine to lose it.
#include "harness.h"

#include "jobs/jobsystem.h"

#include <atomic>
#include <cstdio>
#include <numeric>
#include <set>
#include <vector>

using namespace ely;
using elytest::section;

static void testDispatch() {
    section("dispatch");

    // Zero workers must still run everything, on the calling thread. A caller
    // should never have to ask whether the pool has threads.
    {
        JobSystem jobs(0);
        CHECK(jobs.workerCount() == 0, "asked for 0 workers, got %d",
              jobs.workerCount());
        int ran = 0;
        jobs.dispatch([&] { ++ran; });
        CHECK(ran == 1, "a job dispatched to an empty pool did not run");
    }

    // Every job runs exactly once, and they all finish before the pool is gone.
    {
        constexpr int kJobs = 2000;
        std::atomic<int> counter{0};
        std::vector<std::atomic<int>> touched(kJobs);
        for (auto& t : touched) t.store(0);
        {
            JobSystem jobs(4);
            Latch latch(kJobs);
            for (int i = 0; i < kJobs; ++i) {
                jobs.dispatch([&, i] {
                    touched[size_t(i)].fetch_add(1);
                    counter.fetch_add(1);
                    latch.countDown();
                });
            }
            latch.wait();
        }
        CHECK(counter.load() == kJobs, "%d of %d jobs ran", counter.load(), kJobs);
        bool exactlyOnce = true;
        for (int i = 0; i < kJobs; ++i)
            if (touched[size_t(i)].load() != 1) exactlyOnce = false;
        CHECK(exactlyOnce, "some job ran more than once, or not at all");
    }
}

static void testParallelFor() {
    section("parallelFor");

    for (int workers : {0, 1, 2, 4, 8}) {
        JobSystem jobs(workers);
        constexpr size_t kN = 10000;
        std::vector<int> visits(kN, 0);
        jobs.parallelFor(0, kN, [&](size_t i) { visits[i] += 1; });

        bool once = true;
        for (size_t i = 0; i < kN; ++i)
            if (visits[i] != 1) once = false;
        CHECK(once, "with %d workers, some index was visited a number of times"
              " other than once", workers);
    }

    // An empty or single-element range must not fall over.
    {
        JobSystem jobs(4);
        int ran = 0;
        jobs.parallelFor(5, 5, [&](size_t) { ++ran; });
        CHECK(ran == 0, "an empty range ran %d times", ran);
        jobs.parallelFor(0, 1, [&](size_t) { ++ran; });
        CHECK(ran == 1, "a one-element range ran %d times", ran);
    }

    // Nesting must collapse to serial rather than deadlock. Without the guard
    // in parallelFor, each of N workers tries to occupy N workers and the whole
    // pool waits for capacity it is itself holding — a hang that only appears
    // under load.
    {
        JobSystem jobs(4);
        std::atomic<int> inner{0};
        jobs.parallelFor(0, 64, [&](size_t) {
            jobs.parallelFor(0, 8, [&](size_t) { inner.fetch_add(1); });
        });
        CHECK(inner.load() == 64 * 8,
              "nested parallelFor ran %d of %d iterations", inner.load(), 64 * 8);
        CHECK(jobs.stats().serialFallbacks.load() > 0,
              "nested calls were not detected; they must run serially");
    }

    // Uneven work must not leave threads idle waiting for one unlucky chunk.
    // This is why the loop splits into more chunks than threads and claims them
    // from a shared counter rather than dividing the range up front.
    {
        JobSystem jobs(4);
        constexpr size_t kN = 400;
        std::vector<int> done(kN, 0);
        std::atomic<int> spins{0};
        jobs.parallelFor(0, kN, [&](size_t i) {
            // One index costs far more than the rest.
            const int work = (i == kN / 2) ? 200000 : 100;
            int acc = 0;
            for (int k = 0; k < work; ++k) acc += k & 7;
            spins.fetch_add(acc & 1);
            done[i] = 1;
        }, 1);
        CHECK(std::accumulate(done.begin(), done.end(), 0) == int(kN),
              "an unbalanced parallelFor did not finish every index");
    }
}

static void testDeterminism() {
    section("determinism across thread counts");

    // The property the whole engine rests on: what comes out must not depend on
    // how many threads went in. Each index writes to its own slot and nothing
    // is accumulated across jobs, so this holds by construction — and this test
    // is what catches the day somebody adds a shared accumulator.
    auto compute = [](size_t i) -> double {
        double acc = double(i);
        for (int k = 0; k < 24; ++k) acc = acc * 1.0000001 + 0.5 / (1.0 + acc);
        return acc;
    };

    constexpr size_t kN = 5000;
    std::vector<double> reference(kN);
    {
        JobSystem serial(0);
        serial.parallelFor(0, kN, [&](size_t i) { reference[i] = compute(i); });
    }

    bool identical = true;
    int worstWorkers = 0;
    size_t worstIndex = 0;
    for (int workers : {1, 2, 3, 4, 8, 16}) {
        for (int attempt = 0; attempt < 3; ++attempt) {
            JobSystem jobs(workers);
            std::vector<double> got(kN, 0.0);
            jobs.parallelFor(0, kN, [&](size_t i) { got[i] = compute(i); });
            for (size_t i = 0; i < kN; ++i) {
                // Bit-identical, not approximately equal. Floating-point
                // addition is not associative, so "close enough" is exactly the
                // standard that lets a reordered reduction slip through.
                if (got[i] != reference[i]) {
                    identical = false;
                    worstWorkers = workers;
                    worstIndex = i;
                }
            }
        }
    }
    CHECK(identical, "output differed at index %zu with %d workers; results"
          " must not depend on scheduling", worstIndex, worstWorkers);
}

static void testStress() {
    section("stress");

    // Pools created and destroyed repeatedly, with work in flight. Shutdown
    // races are the classic way a job system passes every test and then hangs
    // once a week on someone's machine.
    for (int round = 0; round < 40; ++round) {
        JobSystem jobs(4);
        std::atomic<int> counter{0};
        Latch latch(200);
        for (int i = 0; i < 200; ++i)
            jobs.dispatch([&] { counter.fetch_add(1); latch.countDown(); });
        latch.wait();
        CHECK_QUIET(counter.load() == 200);
    }
    CHECK(true, "40 rounds of create, saturate and destroy completed");

    // Interleaving dispatch and parallelFor: the loop drains queued jobs while
    // it waits, so the two paths share a queue and must not lose work.
    {
        JobSystem jobs(4);
        std::atomic<int> loose{0};
        Latch latch(500);
        for (int i = 0; i < 500; ++i)
            jobs.dispatch([&] { loose.fetch_add(1); latch.countDown(); });

        std::vector<int> visits(3000, 0);
        jobs.parallelFor(0, visits.size(), [&](size_t i) { visits[i] += 1; });
        latch.wait();

        bool once = true;
        for (int v : visits) if (v != 1) once = false;
        CHECK(once, "parallelFor lost work while draining loose jobs");
        CHECK(loose.load() == 500, "%d of 500 loose jobs ran", loose.load());
    }
}

int main() {
    std::printf("Elysium — jobs tests\n");
    testDispatch();
    testParallelFor();
    testDeterminism();
    testStress();
    return elytest::report("jobs tests");
}
