// Intended function: Track squad/settlement/faction knowledge publication and confidence without omniscience.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct SharedKnowledgeRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SharedKnowledgeRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SharedKnowledgeNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SharedKnowledgeSystem {
public:
 bool submit(const SharedKnowledgeRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SharedKnowledgeRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SharedKnowledgeRecord> snapshot() const;
 std::vector<SharedKnowledgeNotice> drainNotices(); void clear();
private:
 SharedKnowledgeRecord* mutableFind(std::uint64_t); void notice(const SharedKnowledgeRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SharedKnowledgeRecord> records_; std::vector<SharedKnowledgeNotice> notices_;
};
}
