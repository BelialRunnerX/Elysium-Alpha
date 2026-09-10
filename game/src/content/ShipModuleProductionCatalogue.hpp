#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;
enum class ShipSlot:std::uint8_t{Propulsion,Utility,Science,Defense,Control,Industry};
enum class ActivationMode:std::uint8_t{Passive,Toggle,Triggered,Continuous};
enum class AvailabilityStatus:std::uint8_t{Design,Prototype,Merged,Available};
struct ShipModuleProductionRow { ContentId id{}; std::uint32_t schemaVersion{1}; ShipSlot slot{}; std::uint16_t tier{}; double mass{}; double powerDraw{}; std::vector<ContentId> effectRefs; ActivationMode activation{}; ContentId repairRecipe{}; ContentId hardpoint{}; AvailabilityStatus status{AvailabilityStatus::Design}; std::vector<ContentId> acquisitionRefs; };
class ShipModuleProductionCatalogue { public: bool add(ShipModuleProductionRow row,std::string& reason); bool validate(std::string& reason) const; std::optional<ShipModuleProductionRow> get(ContentId id) const; std::vector<ContentId> ids() const; private: std::unordered_map<ContentId,ShipModuleProductionRow> rows_; };
}