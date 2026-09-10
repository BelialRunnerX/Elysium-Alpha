#pragma once

#include "fortress/WorkflowCommon.hpp"

namespace elysium::fortress {

struct ContractProgressInput {
    ContractState contract{};
    float objectiveProgress{};
    bool deadlineExpired{};
    bool complicationFailed{};
};

WorkflowPlan updateContract(const ContractProgressInput& input,
                            std::uint64_t seed,
                            std::uint64_t tick,
                            std::uint32_t producer = 350);

} // namespace elysium::fortress
