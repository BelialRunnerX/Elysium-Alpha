// Intended function: active scanner pulses that convert nearby stable world signals into persistent discovery/knowledge records.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
enum class ScanSignalKind:std::uint8_t{Resource,Lifeform,Structure,Machine,Ship,Anomaly,Rift,Artifact,Beacon,Threat};
struct ScanSignal{std::uint64_t signalId{},targetStableId{},address{};ScanSignalKind kind{};float strength{},difficulty{};};
struct ScanPulse{std::uint64_t pulseId{},scannerId{};float range{},power{};std::uint32_t capabilityMask{~0u};};
struct DiscoveryRecord{std::uint64_t targetStableId{},firstPulseId{};ScanSignalKind kind{};float confidence{};bool identified{};};
class ScanningSystem{public:void upsertSignal(ScanSignal signal);std::vector<DiscoveryRecord>pulse(const ScanPulse&pulse,const std::vector<std::uint64_t>&nearbySignalIds);std::optional<DiscoveryRecord>discovery(std::uint64_t targetId)const;private:std::unordered_map<std::uint64_t,ScanSignal>signals_;std::unordered_map<std::uint64_t,DiscoveryRecord>discoveries_;};
}
