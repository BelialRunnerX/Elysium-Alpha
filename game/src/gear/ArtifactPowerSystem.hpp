// Intended function: Track unique artifact powers, unlock conditions, resonance, costs, and historical provenance.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::gear {
struct ArtifactPowerSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ArtifactPowerSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ArtifactPowerSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ArtifactPowerSystemSystem {
public:
 bool submit(const ArtifactPowerSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ArtifactPowerSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ArtifactPowerSystemRecord> snapshot() const;
 std::vector<ArtifactPowerSystemNotice> drainNotices(); void clear();
private:
 ArtifactPowerSystemRecord* mutableFind(std::uint64_t); void notice(const ArtifactPowerSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ArtifactPowerSystemRecord> records_; std::vector<ArtifactPowerSystemNotice> notices_;
};
}
