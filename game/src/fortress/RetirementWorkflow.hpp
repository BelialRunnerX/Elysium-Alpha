#pragma once

#include "fortress/Retirement.hpp"
#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

WorkflowPlan planFortressRetirement(const SiteSnapshot& snapshot,
                                    TimeStamp now,
                                    std::uint64_t nextHistoryGeneration,
                                    std::uint64_t seed,
                                    std::uint64_t tick,
                                    std::uint32_t producer = 260);
WorkflowPlan planFortressReclamation(const SiteSnapshot& snapshot,
                                     TimeStamp now,
                                     std::uint64_t seed,
                                     std::uint64_t tick,
                                     std::uint32_t producer = 261);

} // namespace elysium::fortress
