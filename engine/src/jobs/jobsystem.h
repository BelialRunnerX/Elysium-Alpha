// The job system: one pool, everything parallel goes through it.
//
// The engine already built tiles on worker threads, with a thread pool owned by
// WorldView. That was fine as far as it went and it went exactly one place —
// nothing else could use it, and anything else that wanted threads would have
// spawned its own and oversubscribed the machine. This is the same idea with
// the ownership inverted: one pool, sized once, and every parallel thing in the
// engine asks it for capacity.
//
// Four decisions, each of which is a thing that goes wrong otherwise.
//
// 1. NOT std::async. The launch policy is implementation-defined — libstdc++
//    will happily run `std::async` deferred, on the calling thread, at the
//    moment you wait for it — there is no pooling, so each call may create and
//    destroy an OS thread, and there is no way to say how many run at once.
//    None of that is acceptable for work that happens every frame.
//
// 2. parallelFor RUNS ON THE CALLING THREAD TOO. It is not "hand it all to the
//    workers and block"; the caller takes a share. With one worker that makes
//    it exactly a serial loop, which is what lets a single-threaded build be a
//    configuration rather than a separate code path.
//
// 3. NESTED parallelFor RUNS SERIALLY. Tile building is already parallel across
//    tiles; if generating a tile's columns also fanned out, N workers would each
//    try to occupy N workers. At best that oversubscribes, at worst every worker
//    is blocked waiting for capacity that every other worker is holding — a
//    deadlock that only appears under load, which is the worst kind. A thread
//    already inside a job runs its loops itself.
//
// 4. RESULTS DO NOT DEPEND ON SCHEDULING. Every job writes to its own slot; the
//    system never accumulates across jobs. Determinism (principle S2) is the
//    property this engine is built around and threading is the easiest way to
//    lose it, so there is a test that the same tiles built on one thread and on
//    eight are byte-identical.
#pragma once
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace ely {

class JobSystem {
public:
    // `workers` is the number of *additional* threads. Zero is legal and means
    // everything runs on the calling thread — the mode the tests use when they
    // want to remove scheduling from the picture entirely.
    //
    // The default leaves one core for the main thread, which on this engine is
    // doing uploads and draw submission and is the thread a frame is measured
    // on.
    explicit JobSystem(int workers = -1);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    int workerCount() const { return int(threads_.size()); }

    // Queue a job. Returns immediately; use a Latch to know when it is done.
    void dispatch(std::function<void()> job);

    // Run `fn(i)` for i in [begin, end), across the pool and the calling thread.
    // Blocks until every index has been run.
    //
    // `grain` is the smallest number of indices worth handing to another thread.
    // Too small and the coordination costs more than the work; the default suits
    // work of roughly a microsecond per index, which is what a terrain column
    // costs.
    void parallelFor(size_t begin, size_t end,
                     const std::function<void(size_t)>& fn,
                     size_t grain = 1);

    // True if the calling thread is one of this system's workers. Used to make
    // nesting collapse to serial rather than deadlock.
    bool onWorkerThread() const;

    // Counters, for anyone who wants to know whether the pool is actually being
    // used. Cheap enough to leave on.
    struct Stats {
        std::atomic<uint64_t> jobsRun{0};
        std::atomic<uint64_t> parallelForCalls{0};
        std::atomic<uint64_t> serialFallbacks{0};   // nested, so run serially
    };
    const Stats& stats() const { return stats_; }

private:
    struct Queue;

    void workerLoop();
    bool popJob(std::function<void()>& out);

    std::vector<std::thread> threads_;
    std::vector<std::function<void()>> queue_;
    size_t head_ = 0;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::atomic<bool> stopping_{false};
    Stats stats_;
};

// A countdown a producer can wait on.
//
// Deliberately not a future: futures allocate a shared state per job and this
// engine dispatches thousands per second. A latch is one atomic and one
// condition variable for a whole batch.
class Latch {
public:
    explicit Latch(size_t count) : remaining_(count) {}

    void countDown(size_t n = 1) {
        if (remaining_.fetch_sub(n) == n) {
            std::lock_guard<std::mutex> lock(mutex_);
            done_.notify_all();
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        done_.wait(lock, [this] { return remaining_.load() == 0; });
    }

    bool ready() const { return remaining_.load() == 0; }

private:
    std::atomic<size_t> remaining_;
    std::mutex mutex_;
    std::condition_variable done_;
};

}  // namespace ely
