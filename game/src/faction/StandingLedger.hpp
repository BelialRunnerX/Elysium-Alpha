#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::faction {
enum class StandingMeter:std::uint8_t{Favor,Suspicion};
struct StandingSignal{std::uint64_t transactionId{},playerId{},systemId{},sourceId{};StandingMeter meter{StandingMeter::Suspicion};double delta{};};
struct StandingSnapshot{std::uint64_t playerId{},systemId{};double favor{},suspicion{},suspicionFloor{};std::uint64_t revision{};};
struct StandingTuning{double minimum{0},maximum{100};double favorDecayPerSecond{},suspicionDecayPerSecond{};};
class StandingLedger{public:explicit StandingLedger(StandingTuning t={});bool apply(const StandingSignal&s,double scale=1);void setClaimFloor(std::uint64_t playerId,std::uint64_t systemId,double floor);void decay(double dt);[[nodiscard]]const StandingSnapshot*find(std::uint64_t playerId,std::uint64_t systemId)const;[[nodiscard]]std::vector<StandingSnapshot>snapshot()const;private:static std::uint64_t key(std::uint64_t playerId,std::uint64_t systemId);StandingTuning tuning_;std::unordered_map<std::uint64_t,StandingSnapshot>rows_;std::unordered_set<std::uint64_t>transactions_;};
} // namespace elysium::faction
