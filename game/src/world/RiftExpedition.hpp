// Intended function: imported world implementation for RiftExpedition; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetTypes.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace elysium {

enum class RiftRoomKind : std::uint8_t { Entrance = 0, Filler = 1, Loot = 2, Boss = 3 };

enum RiftDoor : std::uint8_t {
    RiftDoorNone  = 0,
    RiftDoorNorth = 1u << 0,
    RiftDoorEast  = 1u << 1,
    RiftDoorSouth = 1u << 2,
    RiftDoorWest  = 1u << 3,
};

struct RiftRoom {
    std::uint64_t stableId{};
    std::uint8_t x{};
    std::uint8_t y{};
    std::uint8_t depth{};
    std::uint8_t doors{};
    RiftRoomKind kind{RiftRoomKind::Filler};

    friend bool operator==(const RiftRoom&, const RiftRoom&) = default;
};

struct RiftValidation {
    bool valid{};
    std::string reason;
};

// Deterministic Third-Edition topology contract: 9x9 grid, centered entrance,
// 12 occupied rooms, newest-cell growth bias, BFS depth, symmetric doors,
// deepest boss and two deep dead-end loot rooms.
class RiftTopology {
public:
    static constexpr int GridSize = 9;
    static constexpr int RoomCount = 12;
    static constexpr int LootRoomCount = 2;
    static constexpr std::uint32_t GeneratorVersion = 1;
    static constexpr std::uint64_t GeneratorFingerprint = 0x52494654544F5031ULL; // RIFTTOP1

    static RiftTopology generate(std::uint64_t seed);

    std::uint64_t seed() const { return seed_; }
    const std::vector<RiftRoom>& rooms() const { return rooms_; }
    const RiftRoom* roomAt(int x, int y) const;
    RiftValidation validate() const;
    std::uint64_t fingerprint() const;

private:
    std::uint64_t seed_{};
    std::vector<RiftRoom> rooms_;
};

// Boons are stable run-scoped identities. Their actual passive-hook effects are
// deliberately outside this world-generation module so rules/content can own
// behavior without a name switch in the dungeon generator.
enum class RiftBoon : std::uint8_t {
    VoidEcho = 0,
    PlasmaWake,
    NeuralThread,
    DimensionalStep,
    KineticAnchor,
    SalvagerInstinct,
    SecondWind,
    HunterMark,
    QuietFile,
    DeepScanner,
    EmergencySeal,
    ArtifactSense,
};

struct RiftScalingContext {
    int depth{1};
    int openerLevel{1};
    // Lexicographic rank: advancing one depth always outweighs the bounded
    // player-level minor term. Combat owns conversion from rank to stats.
    int depthDominantRank{16};
};

struct RiftRunResolution {
    int bankedGained{};
    int carriedLost{};
    int persistentSuspicionHeat{}; // caller decides how/if to apply to faction state
    bool ejected{};
    bool gearSurvives{true};
    bool instanceCollapsed{true};
};

class RiftRunState {
public:
    static constexpr std::uint32_t StateVersion = 1;
    static constexpr int PersistentHeatDivisor = 4; // tuning: one quarter of run heat leaves with the player

    static RiftRunState begin(std::uint64_t siteId,
                              std::uint64_t runSeed,
                              int openerLevel,
                              PlanetClass theme);

    std::uint64_t siteId() const { return siteId_; }
    std::uint64_t runId() const { return runId_; }
    std::uint64_t runSeed() const { return runSeed_; }
    int depth() const { return depth_; }
    int openerLevel() const { return openerLevel_; }
    PlanetClass theme() const { return theme_; }
    int carriedValue() const { return carriedValue_; }
    int bankedValue() const { return bankedValue_; }
    int heat() const { return heat_; }
    bool active() const { return active_; }
    const std::vector<RiftBoon>& boons() const { return boons_; }

    RiftTopology topology() const;
    RiftScalingContext scaling() const;

    bool addLoot(int value, int heatAdded = 1);
    bool descend(bool bossDefeated);
    std::array<RiftBoon, 3> boonOffer() const;
    bool acceptBoon(RiftBoon boon);

    RiftRunResolution bankAndExit();
    RiftRunResolution die();

    // Starts a fresh floor-1 expedition while preserving already banked value.
    // A new runId is generated from the deterministic restart sequence.
    void restart();

    std::string serialize() const;
    bool restore(std::string_view text, std::string* error = nullptr);

private:
    std::uint64_t siteId_{};
    std::uint64_t runId_{};
    std::uint64_t runSeed_{};
    std::uint64_t restartSequence_{};
    int depth_{1};
    int openerLevel_{1};
    PlanetClass theme_{PlanetClass::Temperate};
    int carriedValue_{};
    int bankedValue_{};
    int heat_{};
    bool active_{true};
    std::vector<RiftBoon> boons_;

    std::uint64_t floorSeed() const;
    void clearTransientRunState();
};

const char* riftBoonName(RiftBoon boon);

} // namespace elysium
