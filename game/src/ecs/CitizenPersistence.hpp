// Intended function: imported ecs implementation for CitizenPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "ecs/CitizenComponents.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace elysium {

constexpr std::uint32_t kCitizenEntitySchemaVersion = 2;
constexpr std::uint32_t kCitizenEntityOldestReadableSchemaVersion = 1;

std::string serializeRemoteCitizenRecord(const RemoteCitizenRecord& record);
std::optional<RemoteCitizenRecord> deserializeRemoteCitizenRecord(std::string_view text,
                                                                  std::string* error = nullptr);

} // namespace elysium
