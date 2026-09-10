#pragma once
#include <cstdint>
#include <span>
#include <vector>
namespace elysium::base {
struct SensorReport{std::uint64_t sensorId{},contactId{};double confidence{},strength{},distance{};bool hostile{};};
struct DefenseEmplacement{std::uint64_t stableId{},sectorId{};double range{},powerDemand{};std::uint64_t ammo{};bool powered{},enabled{true};};
struct FusedContact{std::uint64_t contactId{};double confidence{},strength{},nearestDistance{};bool hostile{};};
struct DefenseFireRequest{std::uint64_t emplacementId{},targetId{};std::uint64_t ammoUnits{1};double confidence{};};
class BaseDefenseRuntime{public:[[nodiscard]]std::vector<FusedContact>fuse(std::span<const SensorReport>reports)const;[[nodiscard]]std::vector<DefenseFireRequest>planFire(std::span<const DefenseEmplacement>emplacements,std::span<const FusedContact>contacts)const;};
} // namespace elysium::base
