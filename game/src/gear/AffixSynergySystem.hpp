// Intended function: Track interacting item affixes and deterministic synergy activation.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct AffixSynergySystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AffixSynergySystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AffixSynergySystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AffixSynergySystemSystem {
public:
 bool submit(const AffixSynergySystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AffixSynergySystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AffixSynergySystemRecord> snapshot() const;
 std::vector<AffixSynergySystemNotice> drainNotices(); void clear();
private:
 AffixSynergySystemRecord* mutableFind(std::uint64_t); void notice(const AffixSynergySystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AffixSynergySystemRecord> records_; std::vector<AffixSynergySystemNotice> notices_;
};
}
