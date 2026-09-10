#pragma once

#include "fortress/WorkflowCommon.hpp"
#include "fortress/Systems.hpp"

#include <span>

namespace elysium::fortress {

struct HospitalCapacity {
    RoomId hospital{};
    StableId institution{};
    std::uint32_t freeBeds{};
    float cleanliness{1.0f};
    float atmosphereSafety{1.0f};
    float supplyFraction{1.0f};
};

WorkflowPlan planMedicalResponse(StableId patient,
                                 SiteId site,
                                 const BodyState& body,
                                 std::span<const HospitalCapacity> hospitals,
                                 std::uint64_t seed,
                                 std::uint64_t tick,
                                 std::uint32_t producer = 110);

} // namespace elysium::fortress
