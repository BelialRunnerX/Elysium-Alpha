// Intended function: Manage death/knockout recovery, beacon-based respawn choices, inventory penalties, and persistent consequence records.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::gameplay {

struct RespawnOption {
    std::uint64_t siteId{};
    std::uint64_t beaconId{};
    double cost{};
    double risk{};
    double travelDelay{};
    std::uint64_t flags{};
};

class RespawnOptionStore {
public:
    bool upsert(RespawnOption value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const RespawnOption* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<RespawnOption> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const RespawnOption& value) noexcept;
    std::vector<RespawnOption> records_;
};

} // namespace elysium::gameplay
