#include "core/JobSystem.hpp"

namespace elysium {

thread_local bool JobSystem::workerThread_ = false;

void JobSystem::startWorkers(std::size_t count) {
    if (count == 0) { serialMode_ = true; return; }
    workers_.reserve(count);
    for (std::size_t i=0;i<count;++i) workers_.emplace_back([this](){ workerLoop(); });
}

JobSystem::JobSystem(std::size_t requestedWorkers) {
    const unsigned hw = std::thread::hardware_concurrency();
    const std::size_t defaultCount = hw > 1 ? static_cast<std::size_t>(hw - 1) : 1U;
    startWorkers(requestedWorkers == 0 ? defaultCount : requestedWorkers);
}

JobSystem::JobSystem(SerialJobSystemTag) : serialMode_(true) {}
JobSystem::JobSystem(ExactWorkerCount exact) { startWorkers(exact.value); }
JobSystem::~JobSystem() { shutdown(true); }

std::size_t JobSystem::queuedJobs() const { std::lock_guard lock(mutex_); return queue_.size(); }

JobSystemStats JobSystem::stats() const {
    JobSystemStats s;
    s.submitted=submitted_.load(std::memory_order_relaxed);
    s.completed=completed_.load(std::memory_order_relaxed);
    s.active=active_.load(std::memory_order_relaxed);
    { std::lock_guard lock(mutex_); s.queued=queue_.size(); }
    s.workers=workers_.size(); s.stopping=stopping_.load(std::memory_order_relaxed); s.serial=serialMode_;
    return s;
}

void JobSystem::drain() {
    if (serialMode_) return;
    std::unique_lock lock(mutex_);
    drainCv_.wait(lock,[this](){ return queue_.empty() && active_.load(std::memory_order_relaxed)==0; });
}

void JobSystem::shutdown(bool drainPending) {
    if (stopping_.exchange(true,std::memory_order_acq_rel)) return;
    if (!serialMode_) {
        if (!drainPending) { std::lock_guard lock(mutex_); std::queue<std::function<void()>> empty; queue_.swap(empty); }
        cv_.notify_all();
        for(auto& w:workers_) if(w.joinable()) w.join();
        workers_.clear();
    }
    drainCv_.notify_all();
}

bool JobSystem::isWorkerThread() { return workerThread_; }

void JobSystem::workerLoop() {
    workerThread_=true;
    for (;;) {
        std::function<void()> job;
        {
            std::unique_lock lock(mutex_);
            cv_.wait(lock,[this](){ return stopping_.load(std::memory_order_relaxed) || !queue_.empty(); });
            if (queue_.empty() && stopping_.load(std::memory_order_relaxed)) break;
            job=std::move(queue_.front()); queue_.pop();
        }
        job();
    }
    workerThread_=false;
}

} // namespace elysium
