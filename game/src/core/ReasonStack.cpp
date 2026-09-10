#include "core/ReasonStack.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <tuple>

namespace elysium::reason {

ReasonParameter ReasonParameter::unsignedValue(std::uint64_t key, std::uint64_t value) noexcept { return {key, ParameterKind::Unsigned, value}; }
ReasonParameter ReasonParameter::signedValue(std::uint64_t key, std::int64_t value) noexcept { return {key, ParameterKind::Signed, static_cast<std::uint64_t>(value)}; }
ReasonParameter ReasonParameter::scalarValue(std::uint64_t key, double value) noexcept {
    const double safe = safe::finiteClamp(value, -safe::PublishedScalarCeiling, safe::PublishedScalarCeiling);
    return {key, ParameterKind::Scalar, std::bit_cast<std::uint64_t>(safe)};
}
ReasonParameter ReasonParameter::contentId(std::uint64_t key, std::uint64_t value) noexcept { return {key, ParameterKind::ContentId, value}; }
ReasonParameter ReasonParameter::stableId(std::uint64_t key, std::uint64_t value) noexcept { return {key, ParameterKind::StableId, value}; }
double ReasonParameter::scalar() const noexcept { return kind == ParameterKind::Scalar ? std::bit_cast<double>(bits) : 0.0; }

bool ReasonStack::add(ReasonAtom atom) {
    if (atom.code == ReasonCode::None) return false;
    atom.parameterCount = std::min<std::uint8_t>(atom.parameterCount, static_cast<std::uint8_t>(atom.parameters.size()));
    atoms_.push_back(atom);
    normalize();
    return true;
}

void ReasonStack::append(const ReasonStack& other) {
    atoms_.insert(atoms_.end(), other.atoms_.begin(), other.atoms_.end());
    normalize();
}

const ReasonAtom* ReasonStack::primary() const noexcept { return atoms_.empty() ? nullptr : &atoms_.front(); }

void ReasonStack::normalize() {
    std::stable_sort(atoms_.begin(), atoms_.end(), [](const ReasonAtom& a, const ReasonAtom& b) {
        return std::tuple{a.priority, static_cast<std::uint16_t>(a.code), a.providerId, a.subjectId} <
               std::tuple{b.priority, static_cast<std::uint16_t>(b.code), b.providerId, b.subjectId};
    });
    atoms_.erase(std::unique(atoms_.begin(), atoms_.end(), [](const ReasonAtom& a, const ReasonAtom& b) {
        return a.code == b.code && a.providerId == b.providerId && a.subjectId == b.subjectId &&
               a.parameterCount == b.parameterCount && a.parameters == b.parameters;
    }), atoms_.end());
}

std::string_view reasonKey(ReasonCode code) noexcept {
    switch (code) {
        case ReasonCode::None: return "reason.none";
        case ReasonCode::InvalidRequest: return "reason.invalid_request";
        case ReasonCode::DuplicateTransaction: return "reason.duplicate_transaction";
        case ReasonCode::UnknownContent: return "reason.unknown_content";
        case ReasonCode::MissingTool: return "reason.missing_tool";
        case ReasonCode::ToolTierInsufficient: return "reason.tool_tier_insufficient";
        case ReasonCode::ProvenanceForbidden: return "reason.provenance_forbidden";
        case ReasonCode::SourceDepleted: return "reason.source_depleted";
        case ReasonCode::MissingInput: return "reason.missing_input";
        case ReasonCode::MissingCapability: return "reason.missing_capability";
        case ReasonCode::InsufficientPower: return "reason.insufficient_power";
        case ReasonCode::OutputBlocked: return "reason.output_blocked";
        case ReasonCode::ReservationConflict: return "reason.reservation_conflict";
        case ReasonCode::InvalidPlacement: return "reason.invalid_placement";
        case ReasonCode::Collision: return "reason.collision";
        case ReasonCode::UnsupportedSurface: return "reason.unsupported_surface";
        case ReasonCode::SupportUnsafe: return "reason.support_unsafe";
        case ReasonCode::BudgetExceeded: return "reason.budget_exceeded";
        case ReasonCode::InternalInvariant: return "reason.internal_invariant";
    }
    return "reason.unknown";
}

} // namespace elysium::reason
