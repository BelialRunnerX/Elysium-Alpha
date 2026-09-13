#include "automation/ExtractionProvenance.hpp"
#include "core/Saturating.hpp"
#include <algorithm>

namespace elysium::automation {
namespace { constexpr std::uint64_t ProviderExtractor = 0x45585452414354ULL; }

ExtractionEvaluation ExtractionProvenancePolicy::evaluate(const ExtractionRequest& request) const {
    ExtractionEvaluation out{};
    if (request.transactionId == 0 || request.extractorId == 0 || request.sourceDepositId == 0 || request.materialId == 0 || request.requestedUnits == 0) {
        out.reasons.add({reason::ReasonCode::InvalidRequest, ProviderExtractor, request.extractorId, 0});
        return out;
    }
    if (!world::eligibleForNaturalExtractionReward(request.provenance)) {
        out.reasons.add({reason::ReasonCode::ProvenanceForbidden, ProviderExtractor, request.sourceDepositId, 10});
        return out;
    }
    if (request.knownRemainingUnits == 0) {
        out.reasons.add({reason::ReasonCode::SourceDepleted, ProviderExtractor, request.sourceDepositId, 20});
        return out;
    }
    if (request.outputFreeUnits == 0) {
        out.reasons.add({reason::ReasonCode::OutputBlocked, ProviderExtractor, request.extractorId, 30});
        return out;
    }
    out.units = std::min({request.requestedUnits, request.knownRemainingUnits, request.outputFreeUnits});
    out.suspicionSignal = safe::nonNegative(safe::nonNegative(request.suspicionPerUnit) * static_cast<double>(out.units));
    out.accepted = out.units > 0;
    return out;
}

bool DepletionLedger::seedDeposit(std::uint64_t id, std::uint64_t total) {
    if (id == 0) return false;
    auto [it, inserted] = deposits_.try_emplace(id, DepositDepletionState{id, total, 0, 1});
    return inserted;
}
const DepositDepletionState* DepletionLedger::find(std::uint64_t id) const {
    auto it = deposits_.find(id); return it == deposits_.end() ? nullptr : &it->second;
}
bool DepletionLedger::transactionSeen(std::uint64_t id) const { return id != 0 && transactions_.contains(id); }
bool DepletionLedger::commit(const ExtractionRequest& request, const ExtractionEvaluation& evaluation) {
    if (!evaluation.accepted || request.transactionId == 0 || transactions_.contains(request.transactionId)) return false;
    auto it = deposits_.find(request.sourceDepositId);
    if (it == deposits_.end() || it->second.remainingUnits < evaluation.units) return false;
    it->second.remainingUnits -= evaluation.units;
    it->second.extractedUnits = safe::saturatingAdd(it->second.extractedUnits, evaluation.units);
    it->second.revision = safe::saturatingIncrement(it->second.revision);
    transactions_.insert(request.transactionId);
    return true;
}
std::vector<DepositDepletionState> DepletionLedger::snapshot() const {
    std::vector<DepositDepletionState> out; out.reserve(deposits_.size());
    for (const auto& [_, value] : deposits_) out.push_back(value);
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.sourceDepositId < b.sourceDepositId; });
    return out;
}

} // namespace elysium::automation
