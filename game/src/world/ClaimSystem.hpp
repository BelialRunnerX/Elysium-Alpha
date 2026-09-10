// Intended function: stable claim-beacon ownership, contesting and Imperial-exposure events without coupling persistence to transient entities.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class ClaimState:std::uint8_t{Inactive,Establishing,Active,Contested,Disabled,Lost};
struct ClaimRecord{std::uint64_t claimId{},ownerFactionId{},beaconStableId{},siteId{},systemId{};ClaimState state{ClaimState::Inactive};float radiusMeters{},integrity{1},suspicionContribution{};std::uint64_t establishedTick{};};
struct ClaimEvent{std::uint64_t claimId{},systemId{};ClaimState from{},to{};float suspicionDelta{};};
class ClaimRegistry{public:bool create(ClaimRecord record);bool transition(std::uint64_t claimId,ClaimState state,std::uint64_t tick);bool damageBeacon(std::uint64_t claimId,float damage,std::uint64_t tick);std::optional<ClaimRecord>find(std::uint64_t claimId)const;std::vector<ClaimEvent>drainEvents();private:std::unordered_map<std::uint64_t,ClaimRecord>claims_;std::vector<ClaimEvent>events_;};
}
