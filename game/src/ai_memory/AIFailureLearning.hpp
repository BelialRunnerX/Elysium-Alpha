// Intended function: Track recent plan failures and temporarily penalize repeated invalid choices.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct AIFailureLearningRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AIFailureLearningRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AIFailureLearningNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AIFailureLearningSystem {
public:
 bool submit(const AIFailureLearningRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AIFailureLearningRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AIFailureLearningRecord> snapshot() const;
 std::vector<AIFailureLearningNotice> drainNotices(); void clear();
private:
 AIFailureLearningRecord* mutableFind(std::uint64_t); void notice(const AIFailureLearningRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AIFailureLearningRecord> records_; std::vector<AIFailureLearningNotice> notices_;
};
}
