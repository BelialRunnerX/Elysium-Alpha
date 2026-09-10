#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

namespace elysium::fortress {

enum class DiscoveryDisposition : std::uint8_t {
    Private,
    FileEmpire,
    ShareUnsworn
};

struct ScanAttempt {
    StableId scanner{};
    StableId target{};
    ContentId targetType;
    ContentId discovery;
    SiteId site{};
    float scannerGrade{1.0f};
    float rangeFactor{1.0f};
    float targetComplexity{1.0f};
    float stability{1.0f};
    float previousCompleteness{};
};

WorkflowPlan resolveScan(const ScanAttempt& attempt,
                         DiscoveryDisposition disposition,
                         float dt,
                         std::uint64_t tick,
                         std::uint32_t producer = 170);
WorkflowPlan enterDirectOperative(StableId citizen,
                                  SiteId site,
                                  ContentId objective,
                                  AlertLevel alert,
                                  std::uint64_t tick,
                                  std::uint32_t producer = 171);

} // namespace elysium::fortress
