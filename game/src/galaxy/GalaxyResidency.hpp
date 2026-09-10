// Intended function: materialize only systems that are actively loaded or require nearby history state.
#pragma once
#include "galaxy/GalaxyGenerator.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace elysium {

enum class SystemResidencyTier : std::uint8_t {
    AddressOnly = 0,     // deterministic address only; no retained descriptor/history
    HistoryResident = 1,// history/strategic state may be resident; no active local simulation
    Active = 2          // entered/loaded system with active local simulation ownership
};

enum class SystemResidencyReason : std::uint32_t {
    None             = 0,
    DirectLoad       = 1u << 0,
    PlayerPresence   = 1u << 1,
    HistoryProximity = 1u << 2,
    RoutePreview     = 1u << 3,
    ExplicitPin      = 1u << 4,
    BackgroundWork   = 1u << 5
};

constexpr SystemResidencyReason operator|(SystemResidencyReason a, SystemResidencyReason b) noexcept {
    return SystemResidencyReason(std::uint32_t(a) | std::uint32_t(b));
}
constexpr SystemResidencyReason operator&(SystemResidencyReason a, SystemResidencyReason b) noexcept {
    return SystemResidencyReason(std::uint32_t(a) & std::uint32_t(b));
}
constexpr bool any(SystemResidencyReason v) noexcept { return std::uint32_t(v) != 0; }

struct SystemResidencyRecord {
    GalaxySystemId systemId{};
    SystemResidencyTier tier{SystemResidencyTier::AddressOnly};
    SystemResidencyReason reasons{SystemResidencyReason::None};
    std::uint64_t revision{};
    std::uint64_t lastRequiredTick{};
    std::uint64_t lastHistoryTouchTick{};
    bool historyDirty{false};

    auto operator<=>(const SystemResidencyRecord&) const = default;
};

struct SystemResidencyPolicy {
    // History can linger after proximity leaves so travel across a boundary does
    // not churn history state every frame. Active simulation has a shorter tail.
    std::uint64_t historyGraceTicks{12'000};
    std::uint64_t activeGraceTicks{600};
    // Optional safety caps. Zero means this layer imposes no population cap;
    // callers provide sparse bounded neighborhoods from route/history policy.
    std::size_t maxHistoryResident{0};
    std::size_t maxActive{0};
};

struct SystemResidencyDelta {
    GalaxySystemId systemId{};
    SystemResidencyTier before{SystemResidencyTier::AddressOnly};
    SystemResidencyTier after{SystemResidencyTier::AddressOnly};
    std::uint64_t revision{};
};

class GalaxyResidency final {
public:
    explicit GalaxyResidency(SystemResidencyPolicy policy = {});

    // Adds/removes sparse reasons. No descriptor or ECS entity is created here;
    // the caller reacts to returned tier transitions on the owner thread.
    SystemResidencyDelta require(GalaxySystemId system, SystemResidencyReason reason, std::uint64_t tick);
    SystemResidencyDelta release(GalaxySystemId system, SystemResidencyReason reason, std::uint64_t tick);

    // Replace one proximity window atomically. The input should already be a
    // bounded neighborhood supplied by jump/route/region logic; this class never
    // scans the uint32 address space.
    std::vector<SystemResidencyDelta> setHistoryNeighborhood(
        const std::vector<GalaxySystemId>& systems,
        std::uint64_t tick);

    void markHistoryDirty(GalaxySystemId system, std::uint64_t tick);
    void markHistoryPersisted(GalaxySystemId system, std::uint64_t tick);

    // Applies grace periods and caps. Dirty history is never evicted; callers
    // must persist it first and call markHistoryPersisted.
    std::vector<SystemResidencyDelta> collectDemotions(std::uint64_t tick);

    [[nodiscard]] const SystemResidencyRecord* find(GalaxySystemId system) const;
    [[nodiscard]] std::vector<SystemResidencyRecord> snapshot() const { return records_; }
    [[nodiscard]] std::size_t residentCount() const noexcept { return records_.size(); }
    [[nodiscard]] std::size_t activeCount() const noexcept;
    [[nodiscard]] std::size_t historyCount() const noexcept;

private:
    static constexpr SystemResidencyReason ActiveReasons =
        SystemResidencyReason::DirectLoad | SystemResidencyReason::PlayerPresence | SystemResidencyReason::ExplicitPin;
    static constexpr SystemResidencyReason HistoryReasons =
        SystemResidencyReason::HistoryProximity | SystemResidencyReason::RoutePreview | SystemResidencyReason::BackgroundWork;

    static SystemResidencyTier tierFor(SystemResidencyReason reasons) noexcept;
    SystemResidencyRecord* findMutable(GalaxySystemId system);
    SystemResidencyRecord& ensure(GalaxySystemId system);
    SystemResidencyDelta recalc(SystemResidencyRecord& record, std::uint64_t tick);
    bool eraseIfAddressOnly(GalaxySystemId system);

    SystemResidencyPolicy policy_{};
    std::uint64_t nextRevision_{1};
    std::vector<SystemResidencyRecord> records_; // sorted by systemId for deterministic snapshots
    std::vector<GalaxySystemId> historyNeighborhood_; // sorted unique
};

} // namespace elysium
