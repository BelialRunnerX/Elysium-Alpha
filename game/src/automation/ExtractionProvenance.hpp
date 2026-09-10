#pragma once
#include "core/ReasonStack.hpp"
#include "world/ResourceProvenance.hpp"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace elysium::automation {

struct ExtractionRequest {
    std::uint64_t transactionId{};
    std::uint64_t extractorId{};
    std::uint64_t sourceDepositId{};
    std::uint64_t materialId{};
    world::ResourceProvenance provenance{world::ResourceProvenance::Unknown};
    std::uint64_t requestedUnits{};
    std::uint64_t knownRemainingUnits{};
    std::uint64_t outputFreeUnits{};
    double suspicionPerUnit{};
};

struct ExtractionEvaluation {
    bool accepted{};
    reason::ReasonStack reasons;
    std::uint64_t units{};
    double suspicionSignal{};
};

struct DepositDepletionState {
    std::uint64_t sourceDepositId{};
    std::uint64_t remainingUnits{};
    std::uint64_t extractedUnits{};
    std::uint64_t revision{};
};

class ExtractionProvenancePolicy {
public:
    [[nodiscard]] ExtractionEvaluation evaluate(const ExtractionRequest& request) const;
};

class DepletionLedger {
public:
    bool seedDeposit(std::uint64_t sourceDepositId, std::uint64_t totalUnits);
    [[nodiscard]] const DepositDepletionState* find(std::uint64_t sourceDepositId) const;
    [[nodiscard]] bool transactionSeen(std::uint64_t transactionId) const;
    bool commit(const ExtractionRequest& request, const ExtractionEvaluation& evaluation);
    [[nodiscard]] std::vector<DepositDepletionState> snapshot() const;
private:
    std::unordered_map<std::uint64_t, DepositDepletionState> deposits_;
    std::unordered_set<std::uint64_t> transactions_;
};

} // namespace elysium::automation
