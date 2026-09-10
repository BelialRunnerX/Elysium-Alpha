#pragma once

#include "fortress/Commands.hpp"

#include <entt/entt.hpp>

#include <cstddef>
#include <functional>
#include <optional>

namespace elysium::fortress {

struct FortressCommitHooks {
    std::function<void(const FortressCommand&)> externalCommand;
    std::function<void(const HistoricalEvent&)> historyAppend;
};

struct FortressCommitStats {
    std::size_t applied{};
    std::size_t created{};
    std::size_t updated{};
    std::size_t deferredExternal{};
};

entt::entity findEntityByStableId(entt::registry& registry, StableId stableId);
entt::entity createCitizenEntity(entt::registry& registry, const SpawnCitizenCommand& command);
entt::entity createJobEntity(entt::registry& registry, const JobComponent& job);
FortressCommitStats commitFortressCommands(entt::registry& registry,
                                           FortressCommandBuffer& buffer,
                                           const FortressCommitHooks& hooks = {});

} // namespace elysium::fortress
