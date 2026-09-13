#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

namespace elysium::fortress {

WorkflowPlan applySocialEvent(RelationshipEdge relationship,
                              const Thought& thought,
                              const Personality& personality,
                              SiteId site,
                              std::uint64_t tick,
                              std::uint32_t producer = 360);
WorkflowPlan applyGrief(StableId citizen,
                        StableId deceased,
                        float relationshipAffinity,
                        float relationshipFamiliarity,
                        SiteId site,
                        std::uint64_t tick,
                        std::uint32_t producer = 361);

} // namespace elysium::fortress
