// Intended function: Track favors, insults, rescues, betrayals, gifts, and witnessed social events.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ai_memory {
struct SocialMemoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SocialMemoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SocialMemoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SocialMemorySystem {
public:
 bool submit(const SocialMemoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SocialMemoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SocialMemoryRecord> snapshot() const;
 std::vector<SocialMemoryNotice> drainNotices(); void clear();
private:
 SocialMemoryRecord* mutableFind(std::uint64_t); void notice(const SocialMemoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SocialMemoryRecord> records_; std::vector<SocialMemoryNotice> notices_;
};
}
