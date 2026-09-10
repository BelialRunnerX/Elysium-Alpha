// Intended function: Issue deterministic domain-specific random seeds from world/parent stable identities and labels.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::simx {
struct SimulationSeedRegistryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SimulationSeedRegistryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SimulationSeedRegistryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SimulationSeedRegistrySystem {
public:
 bool submit(const SimulationSeedRegistryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SimulationSeedRegistryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SimulationSeedRegistryRecord> snapshot() const;
 std::vector<SimulationSeedRegistryNotice> drainNotices(); void clear();
private:
 SimulationSeedRegistryRecord* mutableFind(std::uint64_t); void notice(const SimulationSeedRegistryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SimulationSeedRegistryRecord> records_; std::vector<SimulationSeedRegistryNotice> notices_;
};
}
