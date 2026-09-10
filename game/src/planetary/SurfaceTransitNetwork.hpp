// Intended function: Track roads, maglev, tunnels, bridges, throughput, damage, and connectivity.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct SurfaceTransitNetworkRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SurfaceTransitNetworkRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SurfaceTransitNetworkNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SurfaceTransitNetworkSystem {
public:
 bool submit(const SurfaceTransitNetworkRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SurfaceTransitNetworkRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SurfaceTransitNetworkRecord> snapshot() const;
 std::vector<SurfaceTransitNetworkNotice> drainNotices(); void clear();
private:
 SurfaceTransitNetworkRecord* mutableFind(std::uint64_t); void notice(const SurfaceTransitNetworkRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SurfaceTransitNetworkRecord> records_; std::vector<SurfaceTransitNetworkNotice> notices_;
};
}
