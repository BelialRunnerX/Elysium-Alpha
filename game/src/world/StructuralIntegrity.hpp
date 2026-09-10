// Intended function: imported world implementation for StructuralIntegrity; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace elysium {

enum class StructuralDamageCause : std::uint8_t {
    ConstructionEdit,
    Removal,
    Explosion,
    VehicleImpact,
    SiegeImpact,
    MegathreatImpact
};

struct StructuralMaterialProfile {
    bool participates{};
    int maxSupportSpan{};
    float blastResistance{};
    bool explicitFoundationCapable{};
    bool rubble{};
};

// Prototype tuning derived from the construction contract. Natural terrain is
// never enrolled merely because its material has a profile: participation is
// gated by PlanetSurface::playerPlaced().
const StructuralMaterialProfile& structuralMaterialProfile(BlockType type);

struct SurfaceCellAddressHash {
    std::size_t operator()(const SurfaceCellAddress& a) const noexcept;
};

struct SurfaceCellAddressLess {
    bool operator()(const SurfaceCellAddress& a,const SurfaceCellAddress& b) const noexcept;
};

enum class StructuralInvalidationDomain : std::uint32_t {
    Navigation      = 1u << 0u,
    Rooms           = 1u << 1u,
    Atmosphere      = 1u << 2u,
    Visibility      = 1u << 3u,
    Infrastructure  = 1u << 4u
};

constexpr std::uint32_t allStructuralInvalidations() {
    return static_cast<std::uint32_t>(StructuralInvalidationDomain::Navigation) |
           static_cast<std::uint32_t>(StructuralInvalidationDomain::Rooms) |
           static_cast<std::uint32_t>(StructuralInvalidationDomain::Atmosphere) |
           static_cast<std::uint32_t>(StructuralInvalidationDomain::Visibility) |
           static_cast<std::uint32_t>(StructuralInvalidationDomain::Infrastructure);
}

struct StructuralCollapseCommand {
    SurfaceCellAddress source{};
    BlockType material{BlockType::Air};
    StructuralDamageCause cause{StructuralDamageCause::Removal};
    std::uint64_t eventSerial{};
    bool presentAsDebris{};
};

struct StructuralEvaluationBatch {
    SurfaceCellAddress origin{};
    int dirtyRadius{};
    StructuralDamageCause cause{StructuralDamageCause::Removal};
    std::uint64_t eventSerial{};
    std::uint64_t sourceWorldRevision{};
    bool boundedByRadius{};
    bool boundedByCellCap{};
    int evaluatedSteps{};
    std::vector<StructuralCollapseCommand> collapse;
};

struct StructuralDebrisEvent {
    SurfaceCellAddress source{};
    std::optional<SurfaceCellAddress> settledAt;
    BlockType sourceMaterial{BlockType::Air};
    std::uint64_t eventSerial{};
};

struct StructuralInvalidationEvent {
    SurfaceCellAddress origin{};
    int radius{};
    std::uint32_t domains{allStructuralInvalidations()};
    std::uint64_t worldRevisionBefore{};
    std::uint64_t worldRevisionAfter{};
    std::uint64_t eventSerial{};
};

struct StructuralCommitResult {
    int removedCells{};
    int rubbleCells{};
    std::vector<StructuralDebrisEvent> debris;
    std::vector<StructuralInvalidationEvent> invalidations;
};

struct StructuralIntegrityTelemetry {
    std::uint64_t dirtyEvents{};
    std::uint64_t stagedTicks{};
    std::uint64_t evaluatedSteps{};
    std::uint64_t stableCells{};
    std::uint64_t collapsedCells{};
    std::uint64_t radiusClips{};
    std::uint64_t cellCapClips{};
};

class SurfaceStructuralIntegritySystem {
public:
    struct Tuning {
        int dirtyRadius{18};
        int maxIslandCells{4096};
        int stepsPerTick{256};
        int maxPresentationDebris{64};
        int rubbleSearchDepth{32};
    };

    SurfaceStructuralIntegritySystem();
    explicit SurfaceStructuralIntegritySystem(Tuning tuning);

    void notifyEdit(const PlanetSurface& world,
                    SurfaceCellAddress origin,
                    StructuralDamageCause cause = StructuralDamageCause::ConstructionEdit,
                    int radius = -1);

    // Explicit roots let future construction/machine systems register anchored
    // beams, foundation blocks, or load-bearing machine foundations without
    // placing structural authority inside this service.
    void addExplicitRoot(SurfaceCellAddress address);
    void removeExplicitRoot(SurfaceCellAddress address);
    bool isExplicitRoot(SurfaceCellAddress address) const;

    // Performs at most stepBudget work. A completed batch contains deterministic
    // owner-thread commands but does not mutate the world. Call commit() before
    // advancing dependent systems.
    std::optional<StructuralEvaluationBatch> update(const PlanetSurface& world,
                                                     int stepBudget = -1);

    StructuralCommitResult commit(PlanetSurface& world,
                                  const StructuralEvaluationBatch& batch) const;

    bool hasPendingWork() const;
    std::size_t pendingEventCount() const;
    StructuralIntegrityTelemetry telemetry() const { return telemetry_; }
    void clear();

private:
    struct FrontierNode {
        SurfaceCellAddress address{};
        int distance{};
    };
    struct CellState {
        SurfaceCellAddress address{};
        int distance{};
        int support{-1};
        bool naturalRoot{};
        bool boundaryRoot{};
        bool explicitRoot{};
    };
    enum class WorkPhase : std::uint8_t { Collect, SeedSupport, PropagateSupport, EmitCollapse };
    struct SupportNode {
        int remaining{};
        std::size_t cellIndex{};
    };
    struct SupportNodeLess {
        bool operator()(const SupportNode& a,const SupportNode& b) const noexcept {
            if(a.remaining!=b.remaining) return a.remaining<b.remaining;
            return a.cellIndex>b.cellIndex;
        }
    };
    struct DirtyWork {
        SurfaceCellAddress origin{};
        int radius{};
        StructuralDamageCause cause{StructuralDamageCause::Removal};
        std::uint64_t serial{};
        std::uint64_t worldRevision{};
        WorkPhase phase{WorkPhase::Collect};
        std::deque<FrontierNode> frontier;
        std::unordered_set<SurfaceCellAddress,SurfaceCellAddressHash> queued;
        std::unordered_map<SurfaceCellAddress,std::size_t,SurfaceCellAddressHash> cellIndex;
        std::vector<CellState> cells;
        std::priority_queue<SupportNode,std::vector<SupportNode>,SupportNodeLess> supportFrontier;
        std::size_t seedCursor{};
        std::size_t emitCursor{};
        bool radiusClipped{};
        bool cellCapClipped{};
        int evaluatedSteps{};
        std::vector<std::size_t> collapseIndices;
    };

    Tuning tuning_{};
    std::deque<DirtyWork> pending_;
    std::unordered_set<SurfaceCellAddress,SurfaceCellAddressHash> explicitRoots_;
    std::uint64_t nextSerial_{1};
    StructuralIntegrityTelemetry telemetry_{};

    static std::vector<SurfaceCellAddress> neighbors(const PlanetSurface& world,SurfaceCellAddress address);
    static bool structuralCell(const PlanetSurface& world,SurfaceCellAddress address);
    bool naturalRoot(const PlanetSurface& world,SurfaceCellAddress address) const;
    void seedWork(const PlanetSurface& world,DirtyWork& work);
    bool stepCollect(const PlanetSurface& world,DirtyWork& work);
    bool stepSeedSupport(const PlanetSurface& world,DirtyWork& work);
    bool stepPropagate(const PlanetSurface& world,DirtyWork& work);
    bool stepEmit(DirtyWork& work);
    static int transferSupport(const CellState& from,const CellState& to);
    std::optional<SurfaceCellAddress> rubbleLanding(const PlanetSurface& world,SurfaceCellAddress source) const;
};

} // namespace elysium
