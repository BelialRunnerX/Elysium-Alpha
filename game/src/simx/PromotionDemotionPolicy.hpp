// Intended function: Decide when aggregate actors/sites become local detailed simulation and vice versa.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct PromotionDemotionPolicyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PromotionDemotionPolicyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PromotionDemotionPolicyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PromotionDemotionPolicySystem {
public:
 bool submit(const PromotionDemotionPolicyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PromotionDemotionPolicyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PromotionDemotionPolicyRecord> snapshot() const;
 std::vector<PromotionDemotionPolicyNotice> drainNotices(); void clear();
private:
 PromotionDemotionPolicyRecord* mutableFind(std::uint64_t); void notice(const PromotionDemotionPolicyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PromotionDemotionPolicyRecord> records_; std::vector<PromotionDemotionPolicyNotice> notices_;
};
}
