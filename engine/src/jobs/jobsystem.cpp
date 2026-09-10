#include "jobsystem.h"

#include <algorithm>

namespace ely {

namespace {

// Which JobSystem, if any, owns the calling thread.
//
// A pointer rather than a bool so that two systems in one process — a test
// harness with its own pool alongside the engine's, which the tests do — each
// answer only for their own threads. Getting this wrong would make a nested
// loop in one system collapse to serial because an unrelated system happened to
// own the thread.
thread_local const JobSystem* tCurrentSystem = nullptr;

int defaultWorkerCount() {
    const unsigned hardware = std::thread::hardware_concurrency();
    if (hardware <= 1) return 0;
    // One fewer than the machine has: the main thread is doing uploads and draw
    // submission, and it is the thread a frame time is measured on. Taking every
    // core for workers makes the number that matters worse.
    return int(hardware) - 1;
}

}  // namespace

JobSystem::JobSystem(int workers) {
    const int count = workers < 0 ? defaultWorkerCount() : workers;
    threads_.reserve(size_t(count > 0 ? count : 0));
    for (int i = 0; i < count; ++i)
        threads_.emplace_back([this] { workerLoop(); });
}

JobSystem::~JobSystem() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    wake_.notify_all();
    for (std::thread& t : threads_)
        if (t.joinable()) t.join();
}

bool JobSystem::onWorkerThread() const { return tCurrentSystem == this; }

void JobSystem::workerLoop() {
    tCurrentSystem = this;
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            wake_.wait(lock, [this] { return stopping_ || head_ < queue_.size(); });
            if (stopping_ && head_ >= queue_.size()) break;
            job = std::move(queue_[head_++]);
            // Reclaim once the queue has drained, rather than erasing from the
            // front every time: erase is linear and this queue sees thousands
            // of jobs a second.
            if (head_ == queue_.size()) {
                queue_.clear();
                head_ = 0;
            }
        }
        job();
        stats_.jobsRun.fetch_add(1, std::memory_order_relaxed);
    }
    tCurrentSystem = nullptr;
}

bool JobSystem::popJob(std::function<void()>& out) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (head_ >= queue_.size()) return false;
    out = std::move(queue_[head_++]);
    if (head_ == queue_.size()) {
        queue_.clear();
        head_ = 0;
    }
    return true;
}

void JobSystem::dispatch(std::function<void()> job) {
    if (threads_.empty()) {
        // No workers: run it here. A caller must never have to ask whether the
        // system has threads before using it.
        job();
        stats_.jobsRun.fetch_add(1, std::memory_order_relaxed);
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(job));
    }
    wake_.notify_one();
}

void JobSystem::parallelFor(size_t begin, size_t end,
                            const std::function<void(size_t)>& fn,
                            size_t grain) {
    stats_.parallelForCalls.fetch_add(1, std::memory_order_relaxed);
    if (begin >= end) return;
    if (grain == 0) grain = 1;

    const size_t total = end - begin;

    // Serial when there is nothing to gain, and — importantly — when this call
    // is already inside a job. Tile building is parallel across tiles; if the
    // columns inside a tile also fanned out, every worker would be waiting for
    // capacity that every other worker is holding.
    if (threads_.empty() || total <= grain || onWorkerThread()) {
        if (onWorkerThread())
            stats_.serialFallbacks.fetch_add(1, std::memory_order_relaxed);
        for (size_t i = begin; i < end; ++i) fn(i);
        return;
    }

    // Split into more chunks than there are threads. Terrain columns are not
    // uniform — one may sit under a mountain and its neighbour under open sky —
    // so an even split by thread count would leave workers idle waiting for the
    // unlucky one. Smaller chunks claimed from a shared counter self-balance.
    const size_t chunkSize = std::max(grain, total / (threads_.size() * 4 + 1));
    const size_t chunks = (total + chunkSize - 1) / chunkSize;

    std::atomic<size_t> next{0};

    // The latch counts RUNNERS, not chunks.
    //
    // Counting chunks looks equivalent and is a use-after-free. The last chunk's
    // countDown releases the caller, which returns and destroys this stack frame
    // — while the worker that ran that chunk is still going round its loop to
    // read `next` and discover there is nothing left. Counting runners means the
    // latch cannot reach zero until every one of them has left the loop and
    // stopped touching anything here.
    const size_t helpers = std::min(threads_.size(), chunks - 1);
    Latch done(helpers + 1);

    auto runChunks = [&]() {
        for (;;) {
            const size_t chunk = next.fetch_add(1, std::memory_order_relaxed);
            if (chunk >= chunks) break;
            const size_t lo = begin + chunk * chunkSize;
            const size_t hi = std::min(lo + chunkSize, end);
            for (size_t i = lo; i < hi; ++i) fn(i);
        }
        done.countDown();
    };

    // Every helper gets a wake-up, and the calling thread joins in rather than
    // blocking. With one worker that makes this a serial loop on the caller,
    // which is exactly what it should be.
    for (size_t i = 0; i < helpers; ++i) dispatch(runChunks);
    runChunks();

    // The caller has run out of chunks but helpers may still be finishing.
    // Rather than sleep, drain the queue: whatever else is pending is work this
    // thread can do, and doing it beats waiting for it.
    while (!done.ready()) {
        std::function<void()> job;
        if (popJob(job)) {
            job();
            stats_.jobsRun.fetch_add(1, std::memory_order_relaxed);
        } else {
            std::this_thread::yield();
        }
    }

    // And then wait properly, even though the spin above says it is ready.
    //
    // ready() reads the counter; countDown decrements the counter and *then*
    // takes the latch's mutex to notify. Between those two the counter says
    // zero while a worker is still inside the latch. wait() takes that same
    // mutex, so returning from it proves the last countDown has finished — and
    // proves it is safe to destroy the latch this frame owns.
    done.wait();
}

}  // namespace ely
