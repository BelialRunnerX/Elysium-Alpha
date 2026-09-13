#include "automation/ExtractionProvenance.hpp"
#include "construction/BlueprintStaging.hpp"
#include "construction/PlacementCatalogue.hpp"
#include "gameplay/MiningTransaction.hpp"
#include "industry/ProcessingRecipeGraph.hpp"
#include <array>
#include <cassert>

void chunk06_contract_tests_unrun() {
    using namespace elysium;
    gameplay::MiningTransactionPolicy mining;
    gameplay::MiningRequest mine{1,2,3,4,5,world::ResourceProvenance::NaturalGenerated,2,1,8.0,4.0,2,1,6,1.5};
    auto m=mining.evaluate(mine); assert(m.accepted && m.requiredWorkSeconds==2.0 && !m.signals.empty());
    mine.provenance=world::ResourceProvenance::PlayerPlaced; auto placed=mining.evaluate(mine); assert(placed.accepted && placed.signals.empty());

    automation::ExtractionProvenancePolicy extractor;
    automation::ExtractionRequest er{11,12,13,14,world::ResourceProvenance::PlayerPlaced,4,20,20,1.0};
    assert(!extractor.evaluate(er).accepted);
    er.provenance=world::ResourceProvenance::NaturalGenerated; auto ee=extractor.evaluate(er); assert(ee.accepted&&ee.units==4);

    industry::ProcessingRecipeGraph graph;
    industry::ProcessingRecipe r{21,industry::OperationKind::Smelt,9,{{1,2}},{{2,1}},3.0,5.0,industry::StatePolicy::RecreateOutput,0.0,1};
    assert(graph.publish(r)); std::array<industry::ItemAmount,1> inv{{{1,2}}};
    assert(graph.plan(21,{9,inv,4,5.0}).accepted);

    construction::PlacementCatalogue pc; construction::PlacementFamily fam{}; fam.familyId=30; fam.allowedModesMask=construction::placementModeBit(construction::PlacementMode::Grid); fam.minimumSupport01=.5; assert(pc.publish(fam));
    assert(pc.evaluate({30,construction::PlacementMode::Grid,false,true,.75}).accepted);

    construction::BlueprintStagingService bs; construction::BlueprintDefinition bd{};bd.blueprintId=40;bd.components.push_back({30,{0,0,0,0},0});bd.costs.push_back({41,2});assert(bs.publish(bd));
    auto plan=bs.create(50,51,40,52,53);assert(plan.accepted&&plan.instance.components.size()==1);
}
