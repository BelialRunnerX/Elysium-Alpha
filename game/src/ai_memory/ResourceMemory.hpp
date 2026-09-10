// Intended function: Track remembered resources, depletion, access, claims, and confidence for AI planning.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct ResourceMemoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ResourceMemoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ResourceMemoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ResourceMemorySystem {
public:
 bool submit(const ResourceMemoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ResourceMemoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ResourceMemoryRecord> snapshot() const;
 std::vector<ResourceMemoryNotice> drainNotices(); void clear();
private:
 ResourceMemoryRecord* mutableFind(std::uint64_t); void notice(const ResourceMemoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ResourceMemoryRecord> records_; std::vector<ResourceMemoryNotice> notices_;
};
}
