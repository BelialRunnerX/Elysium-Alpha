// Intended function: Track toxicity, nutrients, microbes, erosion, amendments, and agricultural readiness.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct SoilReclamationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SoilReclamationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SoilReclamationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SoilReclamationSystem {
public:
 bool submit(const SoilReclamationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SoilReclamationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SoilReclamationRecord> snapshot() const;
 std::vector<SoilReclamationNotice> drainNotices(); void clear();
private:
 SoilReclamationRecord* mutableFind(std::uint64_t); void notice(const SoilReclamationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SoilReclamationRecord> records_; std::vector<SoilReclamationNotice> notices_;
};
}
