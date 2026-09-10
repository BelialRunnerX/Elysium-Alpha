#pragma once
#include "core/ReasonStack.hpp"
#include "world/ResourceProvenance.hpp"
#include <cstdint>
#include <unordered_set>
#include <vector>

namespace elysium::gameplay {

using StableId = std::uint64_t;
using ContentId = std::uint64_t;

struct MiningRequest {
    std::uint64_t transactionId{};
    StableId actorId{};
    StableId toolId{};
    std::uint64_t targetAddressKey{};
    ContentId materialId{};
    world::ResourceProvenance provenance{world::ResourceProvenance::Unknown};
    std::uint32_t toolTier{};
    std::uint32_t requiredToolTier{};
    double hardness{1.0};
    double toolWorkPerSecond{1.0};
    std::uint32_t baseDropCount{1};
    std::uint32_t durabilityCost{1};
    std::uint32_t xpReward{};
    double suspicionSignal{};
};

struct MiningItemGrant { ContentId contentId{}; std::uint32_t count{}; };
struct MiningWorldEdit { std::uint64_t targetAddressKey{}; ContentId expectedMaterialId{}; bool replaceWithAir{true}; };
struct MiningSignal { std::uint64_t topicId{}; StableId sourceId{}; double amount{}; };

struct MiningEvaluation {
    bool accepted{};
    reason::ReasonStack reasons;
    double requiredWorkSeconds{};
    std::uint32_t durabilityCost{};
    std::vector<MiningItemGrant> grants;
    std::vector<MiningSignal> signals;
    MiningWorldEdit worldEdit{};
};

class MiningTransactionPolicy {
public:
    [[nodiscard]] MiningEvaluation evaluate(const MiningRequest& request) const;
};

class MiningTransactionLedger {
public:
    [[nodiscard]] bool seen(std::uint64_t transactionId) const;
    bool markCommitted(std::uint64_t transactionId);
    void clear();
private:
    std::unordered_set<std::uint64_t> committed_;
};

} // namespace elysium::gameplay
