#pragma once
#include "core/Saturating.hpp"
#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace elysium::reason {

enum class ReasonCode : std::uint16_t {
    None = 0,
    InvalidRequest,
    DuplicateTransaction,
    UnknownContent,
    MissingTool,
    ToolTierInsufficient,
    ProvenanceForbidden,
    SourceDepleted,
    MissingInput,
    MissingCapability,
    InsufficientPower,
    OutputBlocked,
    ReservationConflict,
    InvalidPlacement,
    Collision,
    UnsupportedSurface,
    SupportUnsafe,
    BudgetExceeded,
    InternalInvariant
};

enum class ParameterKind : std::uint8_t { Unsigned, Signed, Scalar, ContentId, StableId };

struct ReasonParameter {
    std::uint64_t key{};
    ParameterKind kind{ParameterKind::Unsigned};
    std::uint64_t bits{};

    static ReasonParameter unsignedValue(std::uint64_t key, std::uint64_t value) noexcept;
    static ReasonParameter signedValue(std::uint64_t key, std::int64_t value) noexcept;
    static ReasonParameter scalarValue(std::uint64_t key, double value) noexcept;
    static ReasonParameter contentId(std::uint64_t key, std::uint64_t value) noexcept;
    static ReasonParameter stableId(std::uint64_t key, std::uint64_t value) noexcept;
    [[nodiscard]] double scalar() const noexcept;
    friend bool operator==(const ReasonParameter&, const ReasonParameter&) = default;
};

struct ReasonAtom {
    ReasonCode code{ReasonCode::None};
    std::uint64_t providerId{};
    std::uint64_t subjectId{};
    std::uint16_t priority{};
    std::array<ReasonParameter, 4> parameters{};
    std::uint8_t parameterCount{};
};

class ReasonStack {
public:
    bool add(ReasonAtom atom);
    void append(const ReasonStack& other);
    [[nodiscard]] bool empty() const noexcept { return atoms_.empty(); }
    [[nodiscard]] bool blocked() const noexcept { return !atoms_.empty(); }
    [[nodiscard]] const ReasonAtom* primary() const noexcept;
    [[nodiscard]] std::span<const ReasonAtom> ordered() const noexcept { return atoms_; }
    void clear() noexcept { atoms_.clear(); }
private:
    void normalize();
    std::vector<ReasonAtom> atoms_;
};

[[nodiscard]] std::string_view reasonKey(ReasonCode code) noexcept;

} // namespace elysium::reason
