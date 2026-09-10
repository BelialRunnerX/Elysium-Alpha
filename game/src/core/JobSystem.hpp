#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace elysium {

struct SerialJobSystemTag { explicit constexpr SerialJobSystemTag() = default; };
inline constexpr SerialJobSystemTag SerialJobs{};
struct ExactWorkerCount { std::size_t value{}; };
struct JobSystemStats { std::uint64_t submitted{}, completed{}; std::size_t queued{}, active{}, workers{}; bool stopping{}, serial{}; };

// One engine-wide bounded worker pool. Jobs compute immutable CPU-side results;
// authoritative world/ECS mutation and graphics publication stay on the owner thread.
class JobSystem {
public:
    // Existing constructor keeps automatic-hardware semantics for requestedWorkers==0.
    // Unlike the old implementation, explicit production counts are not arbitrarily capped at 8.
    explicit JobSystem(std::size_t requestedWorkers = 0);
    explicit JobSystem(SerialJobSystemTag);
    // Exact test/tool mode: 0 => serial, otherwise exactly the requested number of workers.
    explicit JobSystem(ExactWorkerCount);
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    std::size_t workerCount() const { return workers_.size(); }
    bool serialMode() const { return serialMode_; }
    std::size_t queuedJobs() const;
    [[nodiscard]] JobSystemStats stats() const;
    void drain();
    void shutdown(bool drainPending = true);

    template <class Fn>
    auto submit(Fn&& fn) -> std::future<std::invoke_result_t<Fn>> {
        using Result = std::invoke_result_t<Fn>;
        auto task = std::make_shared<std::packaged_task<Result()>>(std::forward<Fn>(fn));
        auto future = task->get_future();
        if (serialMode_) {
            if (stopping_.load(std::memory_order_acquire)) throw std::runtime_error("JobSystem is stopping");
            submitted_.fetch_add(1, std::memory_order_relaxed);
            active_.fetch_add(1, std::memory_order_relaxed);
            (*task)();
            active_.fetch_sub(1, std::memory_order_relaxed);
            completed_.fetch_add(1, std::memory_order_relaxed);
            drainCv_.notify_all();
            return future;
        }
        {
            std::lock_guard lock(mutex_);
            if (stopping_.load(std::memory_order_relaxed)) throw std::runtime_error("JobSystem is stopping");
            queue_.emplace([this, task]() {
                active_.fetch_add(1, std::memory_order_relaxed);
                (*task)();
                active_.fetch_sub(1, std::memory_order_relaxed);
                completed_.fetch_add(1, std::memory_order_relaxed);
                drainCv_.notify_all();
            });
            submitted_.fetch_add(1, std::memory_order_relaxed);
        }
        cv_.notify_one();
        return future;
    }

    // Deterministic partition primitive. Partition boundaries and indices depend only
    // on count/grain, never worker count. Scheduling may vary; authoritative publication
    // must merge results in partitionIndex/order after this computation phase.
    template <class Fn>
    void parallelForPartitions(std::size_t count, std::size_t grain, Fn&& fn) {
        if (count == 0) return;
        grain = std::max<std::size_t>(1, grain);
        const std::size_t partitionCount = (count + grain - 1) / grain;
        if (serialMode_ || workers_.size() <= 1 || isWorkerThread() || partitionCount <= 1) {
            for (std::size_t p=0;p<partitionCount;++p) {
                const auto begin=p*grain, end=std::min(count,begin+grain);
                fn(begin,end,p);
            }
            return;
        }
        std::atomic<std::size_t> next{0};
        const std::size_t taskCount = std::min<std::size_t>(workers_.size(), partitionCount);
        auto worker=[&]() {
            for (;;) {
                const auto p=next.fetch_add(1,std::memory_order_relaxed);
                if (p>=partitionCount) return;
                const auto begin=p*grain, end=std::min(count,begin+grain);
                fn(begin,end,p);
            }
        };
        std::vector<std::future<void>> futures;
        futures.reserve(taskCount>0?taskCount-1:0);
        for(std::size_t t=1;t<taskCount;++t) futures.push_back(submit(worker));
        worker();
        for(auto& f:futures) f.get();
    }

    template <class Fn>
    void parallelFor(std::size_t count, Fn&& fn, std::size_t grain = 1) {
        parallelForPartitions(count,grain,[&](std::size_t begin,std::size_t end,std::size_t){
            for(std::size_t i=begin;i<end;++i) fn(i);
        });
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable drainCv_;
    std::queue<std::function<void()>> queue_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stopping_{false};
    bool serialMode_{};
    std::atomic<std::size_t> active_{0};
    std::atomic<std::uint64_t> submitted_{0}, completed_{0};

    void startWorkers(std::size_t count);
    void workerLoop();
    static bool isWorkerThread();
    static thread_local bool workerThread_;
};

} // namespace elysium
