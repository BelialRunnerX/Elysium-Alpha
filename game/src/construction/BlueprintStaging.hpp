#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace elysium::construction {
using StableId = std::uint64_t;
using ContentId = std::uint64_t;

struct RelativePlacement { std::int64_t x{}, y{}, z{}; std::uint16_t rotationStep{}; };
struct BlueprintComponent { ContentId pieceFamilyId{}; RelativePlacement placement{}; std::uint32_t buildOrder{}; };
struct MaterialRequirement { ContentId materialId{}; std::uint64_t amount{}; };
struct BlueprintDefinition {
    ContentId blueprintId{};
    std::uint32_t schemaVersion{1};
    std::vector<BlueprintComponent> components;
    std::vector<MaterialRequirement> costs;
};

enum class BuildStage : std::uint8_t { Ghost, Reserved, Foundation, Framed, Installed, Commissioned };
struct BuildComponentState { std::uint32_t componentIndex{}; BuildStage stage{BuildStage::Ghost}; double progress01{}; };
struct BlueprintInstance {
    StableId instanceId{};
    ContentId blueprintId{};
    StableId ownerId{};
    std::uint64_t reservationId{};
    std::uint64_t revision{1};
    std::vector<BuildComponentState> components;
};
struct BlueprintReservationRequest { std::uint64_t reservationId{}; StableId ownerId{}; std::vector<MaterialRequirement> materials; };
struct BlueprintCreatePlan { bool accepted{}; reason::ReasonStack reasons; BlueprintInstance instance; BlueprintReservationRequest reservation; };

class BlueprintStagingService {
public:
    bool publish(BlueprintDefinition definition, reason::ReasonStack* reasons = nullptr);
    [[nodiscard]] const BlueprintDefinition* find(ContentId blueprintId) const;
    [[nodiscard]] BlueprintCreatePlan create(std::uint64_t transactionId, StableId instanceId, ContentId blueprintId, StableId ownerId, std::uint64_t reservationId) const;
    bool commitCreate(std::uint64_t transactionId, BlueprintInstance instance);
    bool applyProgress(std::uint64_t transactionId, StableId instanceId, std::uint32_t componentIndex, BuildStage stage, double progress01);
    [[nodiscard]] const BlueprintInstance* instance(StableId id) const;
private:
    std::unordered_map<ContentId, BlueprintDefinition> definitions_;
    std::unordered_map<StableId, BlueprintInstance> instances_;
    std::unordered_set<std::uint64_t> transactions_;
};

} // namespace elysium::construction
