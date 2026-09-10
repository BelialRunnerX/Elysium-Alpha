#include "gameplay/MiningTransaction.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>

namespace elysium::gameplay {
namespace {
constexpr std::uint64_t ProviderMining = 0x4d494e494e47ULL;
constexpr std::uint64_t TopicMiningXp = 0x5850ULL;
constexpr std::uint64_t TopicIndustrialSuspicion = 0x53555350ULL;
constexpr std::uint64_t ParamRequiredTier = 1;
constexpr std::uint64_t ParamToolTier = 2;
}

MiningEvaluation MiningTransactionPolicy::evaluate(const MiningRequest& request) const {
    MiningEvaluation out{};
    out.worldEdit = {request.targetAddressKey, request.materialId, true};
    if (request.transactionId == 0 || request.actorId == 0 || request.targetAddressKey == 0 || request.materialId == 0) {
        out.reasons.add({reason::ReasonCode::InvalidRequest, ProviderMining, request.actorId, 0});
        return out;
    }
    if (request.toolId == 0 && request.requiredToolTier > 0) {
        out.reasons.add({reason::ReasonCode::MissingTool, ProviderMining, request.actorId, 10});
    }
    if (request.toolTier < request.requiredToolTier) {
        reason::ReasonAtom atom{reason::ReasonCode::ToolTierInsufficient, ProviderMining, request.actorId, 20};
        atom.parameters[0] = reason::ReasonParameter::unsignedValue(ParamRequiredTier, request.requiredToolTier);
        atom.parameters[1] = reason::ReasonParameter::unsignedValue(ParamToolTier, request.toolTier);
        atom.parameterCount = 2;
        out.reasons.add(atom);
    }
    if (out.reasons.blocked()) return out;

    const double hardness = safe::nonNegative(request.hardness);
    const double workRate = std::max(1e-6, safe::nonNegative(request.toolWorkPerSecond));
    out.requiredWorkSeconds = safe::nonNegative(hardness / workRate);
    out.durabilityCost = request.toolId == 0 ? 0u : request.durabilityCost;
    const std::uint32_t drops = request.baseDropCount;
    if (drops > 0) out.grants.push_back({request.materialId, drops});

    // Natural-source rewards are provenance-gated. Player construction can still
    // return its material but cannot be cycled for mining XP/territorial signals.
    if (world::eligibleForNaturalExtractionReward(request.provenance)) {
        if (request.xpReward > 0) out.signals.push_back({TopicMiningXp, request.actorId, static_cast<double>(request.xpReward)});
        const double suspicion = safe::nonNegative(request.suspicionSignal);
        if (suspicion > 0.0) out.signals.push_back({TopicIndustrialSuspicion, request.actorId, suspicion});
    }
    out.accepted = true;
    return out;
}

bool MiningTransactionLedger::seen(std::uint64_t transactionId) const { return transactionId != 0 && committed_.contains(transactionId); }
bool MiningTransactionLedger::markCommitted(std::uint64_t transactionId) {
    if (transactionId == 0) return false;
    return committed_.insert(transactionId).second;
}
void MiningTransactionLedger::clear() { committed_.clear(); }

} // namespace elysium::gameplay
