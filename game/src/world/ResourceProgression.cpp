// Intended function: imported world implementation for ResourceProgression; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/ResourceProgression.hpp"

#include "world/SurfaceIndustry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::array<OreProgressionDefinition,19> kOreSpine{{
    {"elysium:ore/coal",       "Coal",        40,220,1,"-",           "fuel/carbon", true, BlockType::CoalOre},
    {"elysium:ore/copper",    "Copper",     -20,140,1,"Plasma",      "bronze/brass/conduction", true, BlockType::CopperOre},
    {"elysium:ore/tin",       "Tin",        -30,120,1,"Neural",      "bronze", true, BlockType::TinOre},
    {"elysium:ore/zinc",      "Zinc",       -40,120,1,"Plasma",      "brass/flux", false, BlockType::Air},
    {"elysium:ore/iron",      "Iron",       -60,180,2,"Kinetic",     "steel/general industry", true, BlockType::IronOre},
    {"elysium:ore/bauxite",   "Bauxite",     20,100,2,"Dimensional", "aluminium", false, BlockType::Air},
    {"elysium:ore/resonant_dust","Resonant Dust",-300,200,2,"Neural","power/signal medium", false, BlockType::Air},
    {"elysium:ore/lead",      "Lead",       -90,120,2,"Kinetic",     "radiation shielding", false, BlockType::Air},
    {"elysium:ore/silver",    "Silver",    -110,110,3,"Neural",      "electronics/scanners", false, BlockType::Air},
    {"elysium:ore/nickel",    "Nickel",    -130,110,3,"Kinetic",     "alloys", false, BlockType::Air},
    {"elysium:ore/gold",      "Gold",      -160,150,3,"Neural",      "electronics/trade", false, BlockType::Air},
    {"elysium:ore/cobalt",    "Cobalt",    -200,100,3,"Dimensional", "high-temp alloying", false, BlockType::Air},
    {"elysium:ore/emerald",   "Emerald",    200,120,3,"Neural",      "optics/trade", false, BlockType::Air},
    {"elysium:ore/titanium",  "Titanium",  -230,110,4,"Dimensional", "hulls/pressure systems", false, BlockType::Air},
    {"elysium:ore/tungsten",  "Tungsten",  -280,100,4,"Kinetic",     "thermal shielding", false, BlockType::Air},
    {"elysium:ore/platinum",  "Platinum",  -300, 90,4,"Neural",      "catalysts/warp", false, BlockType::Air},
    {"elysium:ore/diamond",   "Diamond",   -380,130,4,"Dimensional", "tools/pressure lattice", false, BlockType::Air},
    {"elysium:ore/osmium",    "Osmium",    -400, 90,4,"Void",        "radiation cores", false, BlockType::Air},
    {"elysium:ore/uranium",   "Uranium",   -420,100,4,"Void",        "reactor fuel", false, BlockType::Air},
}};

// Exotic materials are intentionally catalogued separately because the design
// classifies Voidglass/Aetherium as special deposits/materials and Neutronium as
// a tier-5 mantle-adjacent resource. They still participate in the same public
// oreProgressionCatalog() view below via kAllOreSpine.
constexpr std::array<OreProgressionDefinition,3> kExoticSpine{{
    {"elysium:ore/voidglass",  "Voidglass",  -150,150,3,"Void",        "runes/exotic structures", false, BlockType::Air},
    {"elysium:ore/aetherium",  "Aetherium",  -300,130,4,"Dimensional", "reforge/warp/ascension", false, BlockType::Air},
    {"elysium:ore/neutronium", "Neutronium", -470, 60,5,"Kinetic",     "endgame tooling/structures", false, BlockType::Air},
}};

const std::vector<OreProgressionDefinition>& allOreSpine() {
    static const std::vector<OreProgressionDefinition> all=[] {
        std::vector<OreProgressionDefinition> out;
        out.reserve(kOreSpine.size()+kExoticSpine.size());
        out.insert(out.end(),kOreSpine.begin(),kOreSpine.end());
        out.insert(out.end(),kExoticSpine.begin(),kExoticSpine.end());
        return out;
    }();
    return all;
}

bool contains(const std::unordered_set<int>& set,int value) {
    return set.find(value)!=set.end();
}

} // namespace

std::span<const OreProgressionDefinition> oreProgressionCatalog() {
    const auto& all=allOreSpine();
    return {all.data(),all.size()};
}

const OreProgressionDefinition* oreProgression(std::string_view contentId) {
    const auto all=oreProgressionCatalog();
    const auto it=std::find_if(all.begin(),all.end(),[&](const auto& d){return d.contentId==contentId;});
    return it==all.end()?nullptr:&*it;
}

MiningBreakRule evaluateMiningBreak(BlockType type,int toolTier,bool playerPlaced) {
    const auto& props=blockProperties(type);
    MiningBreakRule out;
    out.mineable=props.mineable;
    out.playerPlaced=playerPlaced;
    out.requiredToolTier=props.harvestTier;
    out.toolAllowed=props.mineable && toolTier>=props.harvestTier;
    if(!out.toolAllowed) return out;

    out.breakSeconds=std::max(0.18f,props.hardness*0.32f/(1.0f+0.25f*static_cast<float>(std::max(1,toolTier)-1)));
    out.drop=miningDrop(type);
    out.dropCount=out.drop==BlockType::Air?0:1;
    // Prototype XP is deliberately small and data-shaped. It is only awarded
    // for successful natural extraction, never for recycling player placement.
    out.xp=playerPlaced?0:std::max(1,props.harvestTier+1);
    out.suspicion=playerPlaced?0.0f:props.suspicionOnMine;
    return out;
}

ObtainabilityReport validateObtainability(std::span<const int> sources,
                                          std::span<const ObtainabilityRecipe> recipes,
                                          std::span<const int> requiredSurvivalItems) {
    ObtainabilityReport report;
    std::unordered_set<int> obtainableSet;
    for(const int item:sources) if(item!=0) obtainableSet.insert(item);

    bool changed=true;
    while(changed) {
        changed=false;
        for(const auto& recipe:recipes) {
            const bool ready=std::all_of(recipe.inputs.begin(),recipe.inputs.end(),[&](int item){return contains(obtainableSet,item);});
            if(!ready) continue;
            for(const int output:recipe.outputs) {
                if(output!=0 && obtainableSet.insert(output).second) changed=true;
            }
        }
    }

    report.obtainable.assign(obtainableSet.begin(),obtainableSet.end());
    std::sort(report.obtainable.begin(),report.obtainable.end());
    for(const int required:requiredSurvivalItems) {
        if(!contains(obtainableSet,required)) report.unreachableRequired.push_back(required);
    }
    std::sort(report.unreachableRequired.begin(),report.unreachableRequired.end());
    report.unreachableRequired.erase(std::unique(report.unreachableRequired.begin(),report.unreachableRequired.end()),report.unreachableRequired.end());

    // Build the unresolved recipe dependency graph. An edge A->B means recipe A
    // needs an item produced by unresolved recipe B. Strongly self-dependent
    // components are reported as circular islands instead of a vague missing-item
    // error.
    std::vector<std::size_t> unresolved;
    std::unordered_map<int,std::vector<std::size_t>> producers;
    for(std::size_t i=0;i<recipes.size();++i) {
        bool outputReachable=false;
        for(const int output:recipes[i].outputs) if(contains(obtainableSet,output)) outputReachable=true;
        if(outputReachable) continue;
        unresolved.push_back(i);
        for(const int output:recipes[i].outputs) producers[output].push_back(i);
    }

    std::unordered_map<std::size_t,std::vector<std::size_t>> edges;
    for(const auto i:unresolved) {
        for(const int input:recipes[i].inputs) {
            if(contains(obtainableSet,input)) continue;
            const auto pit=producers.find(input);
            if(pit!=producers.end()) edges[i].insert(edges[i].end(),pit->second.begin(),pit->second.end());
        }
    }

    // Small deterministic DFS cycle detector. We canonicalize each discovered
    // cycle by sorted recipe IDs so diagnostics are independent of hash order.
    std::vector<int> state(recipes.size(),0);
    std::vector<std::size_t> stack;
    std::set<std::string> cycleKeys;
    const auto dfs=[&](auto&& self,std::size_t node)->void {
        state[node]=1;
        stack.push_back(node);
        auto next=edges[node];
        std::sort(next.begin(),next.end());
        next.erase(std::unique(next.begin(),next.end()),next.end());
        for(const auto other:next) {
            if(state[other]==0) self(self,other);
            else if(state[other]==1) {
                const auto it=std::find(stack.begin(),stack.end(),other);
                if(it!=stack.end()) {
                    std::vector<std::string> ids;
                    for(auto p=it;p!=stack.end();++p) ids.emplace_back(recipes[*p].id);
                    std::sort(ids.begin(),ids.end());
                    std::ostringstream key;
                    for(std::size_t k=0;k<ids.size();++k) { if(k) key<<" -> "; key<<ids[k]; }
                    cycleKeys.insert(key.str());
                }
            }
        }
        stack.pop_back();
        state[node]=2;
    };
    for(const auto i:unresolved) if(state[i]==0) dfs(dfs,i);
    report.circularRecipeIslands.assign(cycleKeys.begin(),cycleKeys.end());

    for(const int item:report.unreachableRequired)
        report.diagnostics.push_back("required survival item unreachable: "+std::to_string(item));
    for(const auto& cycle:report.circularRecipeIslands)
        report.diagnostics.push_back("circular recipe island: "+cycle);

    report.valid=report.unreachableRequired.empty() && report.circularRecipeIslands.empty();
    return report;
}

std::vector<ObtainabilityRecipe> survivalCraftingObtainabilityRecipes() {
    return {
        {"elysium:craft/mining_head_bronze",
         {static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::CopperOre),static_cast<int>(BlockType::TinOre)},
         {static_cast<int>(ProgressionUnlockId::BronzeMiningHead)}},
        {"elysium:craft/mining_head_steel",
         {static_cast<int>(ProgressionUnlockId::BronzeMiningHead),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::IronOre),static_cast<int>(BlockType::CoalOre),static_cast<int>(BlockType::CoalOre)},
         {static_cast<int>(ProgressionUnlockId::SteelMiningHead)}}
    };
}

std::vector<ObtainabilityRecipe> surfaceIndustryObtainabilityRecipes() {
    std::vector<ObtainabilityRecipe> out;
    for(int raw=static_cast<int>(SurfaceRecipeId::SmeltCopper);
        raw<=static_cast<int>(SurfaceRecipeId::FabricateCompositePanel);++raw) {
        const auto id=static_cast<SurfaceRecipeId>(raw);
        const auto* recipe=surfaceRecipe(id);
        if(!recipe) continue;
        ObtainabilityRecipe converted;
        converted.id="elysium:process/surface_"+std::to_string(raw);
        for(int i=0;i<recipe->inputCount;++i) converted.inputs.push_back(recipe->inputs[static_cast<std::size_t>(i)].itemId);
        converted.outputs.push_back(recipe->outputItemId);
        out.push_back(std::move(converted));
    }
    return out;
}

} // namespace elysium
