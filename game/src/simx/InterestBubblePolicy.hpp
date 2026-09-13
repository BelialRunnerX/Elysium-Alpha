// Intended function: Select detailed simulation bubbles around players, cameras, settlements, combat, and critical sites.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct InterestBubblePolicyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct InterestBubblePolicyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct InterestBubblePolicyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class InterestBubblePolicySystem {
public:
 bool submit(const InterestBubblePolicyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const InterestBubblePolicyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<InterestBubblePolicyRecord> snapshot() const;
 std::vector<InterestBubblePolicyNotice> drainNotices(); void clear();
private:
 InterestBubblePolicyRecord* mutableFind(std::uint64_t); void notice(const InterestBubblePolicyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<InterestBubblePolicyRecord> records_; std::vector<InterestBubblePolicyNotice> notices_;
};
}
