#pragma once
#include "combat/TerrainDamageAdapter.hpp"
#include <array>
#include <map>
#include <set>
#include <vector>
namespace elysium::combat {
struct TerrainCellView {
    std::uint64_t address{}, revision{}, material{};
    unsigned minimumToolTier{}; double blastResistance{};
    bool unbreakable{}, fragileDetail{}, protectedByPolicy{};
};
struct TerrainCategoryPolicy { unsigned maxCells{}, minimumToolTier{}; double maximumDamage{}; };
struct ImpactInput {
    TerrainHitContext impact; unsigned toolTier{};
    std::vector<TerrainCellView> cells;
};
enum class TerrainImpactReason { Ready, Invalid, Duplicate, Budget, Protected, Material, Cosmetic, Empty };
struct TerrainEditCommand {
    std::uint64_t event{}, actor{}, address{}, expectedRevision{}, material{};
    double damage{}; TerrainDamageCategory category{}; bool persistent{}, microdetailOnly{};
};
struct TerrainImpactPlan {
    std::uint64_t event{}; TerrainImpactReason reason{TerrainImpactReason::Empty};
    std::vector<TerrainEditCommand> edits;
    std::vector<std::uint64_t> cosmeticMarks, rejectedCells;
};
struct TerrainImpactSnapshot { std::uint64_t tick{}; unsigned spent{}; std::vector<std::uint64_t> accepted; };
// Budget reservation and provenance only. The world-edit owner must validate
// revisions/materials again before committing the returned commands.
class TerrainImpactCommands {
public:
    explicit TerrainImpactCommands(unsigned perTickBudget=128):budget_(perTickBudget){}
    bool setPolicy(TerrainDamageCategory,TerrainCategoryPolicy);
    bool beginTick(std::uint64_t);
    TerrainImpactPlan submit(const ImpactInput&);
    TerrainImpactSnapshot snapshot() const;
    bool restore(const TerrainImpactSnapshot&);
private:
    TerrainDamageAdapter adapter_;
    unsigned budget_,spent_{};std::uint64_t tick_{};
    std::set<std::uint64_t> accepted_;
    std::map<TerrainDamageCategory,TerrainCategoryPolicy> policies_{
        {TerrainDamageCategory::FragileMicrodetail,{4,0,10}},
        {TerrainDamageCategory::Excavation,{32,1,1000}},
        {TerrainDamageCategory::StructuralBreach,{16,2,500}}};
};
}
