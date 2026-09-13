#pragma once

#include "fortress/WorkflowCommon.hpp"

#include <span>

namespace elysium::fortress {

struct TradeLine {
    StableId item{};
    StableId seller{};
    StableId buyer{};
    ContentId content;
    float quantity{};
    float unitPrice{};
};

struct CaravanTransaction {
    StableId caravan{};
    SiteId site{};
    OrganizationId owner{};
    std::vector<TradeLine> lines;
    float creditsTransferred{};
};

WorkflowPlan settleCaravanTrade(const CaravanTransaction& transaction,
                                std::uint64_t seed,
                                std::uint64_t tick,
                                std::uint32_t producer = 210);

} // namespace elysium::fortress
