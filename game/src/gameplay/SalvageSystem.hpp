// Intended function: deterministic salvage/deconstruction yields with condition, skill and provenance hooks for derelicts, ruins, machines and wrecks.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
struct SalvageInput{std::uint64_t sourceStableId{},eventOrdinal{};std::uint32_t sourceType{},materialId{};float condition{1},skill{},toolQuality{};};
struct SalvageYield{std::uint32_t itemId{},quantity{};float condition{};std::uint64_t provenanceId{};};
std::vector<SalvageYield>rollSalvage(std::uint64_t worldSeed,const SalvageInput&input);
}
