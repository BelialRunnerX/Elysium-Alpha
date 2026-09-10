// Intended function: imported world implementation for Derelict; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PoiSites.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

enum class DerelictSecurityState : std::uint8_t { Offline = 0, Dormant = 1, Alert = 2, LockedDown = 3 };

enum DerelictHazard : std::uint32_t {
    DerelictHazardNone = 0,
    DerelictHazardVacuum = 1u << 0,
    DerelictHazardFire = 1u << 1,
    DerelictHazardContamination = 1u << 2,
    DerelictHazardRadiation = 1u << 3,
    DerelictHazardZeroGravity = 1u << 4,
    DerelictHazardSecurity = 1u << 5,
};

struct DerelictSection {
    std::uint64_t stableId{};
    std::uint8_t index{};
    bool powered{};
    float pressure{};      // 0..1
    float gravity{};       // 0..1
    float fire{};          // 0..1
    float contamination{}; // 0..1
    DerelictSecurityState security{DerelictSecurityState::Offline};
    bool bulkheadOpen{true};
    bool emergencySeal{};
    int salvageMass{};
    int salvagedMass{};
    std::uint64_t historicalCrewId{};
    std::uint64_t historicalOwnerId{};

    int remainingSalvageMass() const { return std::max(0, salvageMass - salvagedMass); }
};

struct DerelictLink {
    std::uint8_t a{};
    std::uint8_t b{};
    std::uint64_t stableDoorId{};
};

struct DerelictTraversalPolicy {
    bool vacuumProtection{};
    bool fireProtection{};
    bool contaminationProtection{};
    bool radiationProtection{};
    bool zeroGravityMobility{};
    bool securityAccess{};
};

struct DerelictInspection {
    std::uint64_t siteId{};
    int sections{};
    int poweredSections{};
    int pressurizedSections{};
    int burningSections{};
    int contaminatedSections{};
    int lockedSections{};
    int salvageMassRemaining{};
};

class DerelictState {
public:
    static DerelictState generate(const SiteDescriptor& site, int sectionCount = 8);

    std::uint64_t siteId() const { return siteId_; }
    const std::vector<DerelictSection>& sections() const { return sections_; }
    const std::vector<DerelictLink>& links() const { return links_; }
    DerelictSection* section(std::uint8_t index);
    const DerelictSection* section(std::uint8_t index) const;

    bool setPowered(std::uint8_t index, bool powered);
    bool setBulkhead(std::uint8_t a, std::uint8_t b, bool open);
    bool emergencySeal(std::uint8_t index, bool sealed);
    int salvage(std::uint8_t index, int requestedMass);
    void update(float dt);

    std::vector<std::uint8_t> reachable(std::uint8_t start, const DerelictTraversalPolicy& policy) const;
    std::uint32_t hazards(std::uint8_t index) const;
    DerelictInspection inspect() const;

    std::string serialize() const;
    bool restore(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t siteId_{};
    std::vector<DerelictSection> sections_;
    std::vector<DerelictLink> links_;
    std::map<std::uint64_t, bool> doorOpen_;

    bool passable(std::uint8_t from, std::uint8_t to, const DerelictTraversalPolicy& policy) const;
};

} // namespace elysium
