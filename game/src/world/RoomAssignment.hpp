// Intended function: Track stable room identities, ownership, functional tags, capacity, and reassignment requests for fortress spaces.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct RoomAssignment {
    std::uint64_t roomId{};
    std::uint64_t ownerId{};
    std::uint64_t functionId{};
    double capacity{};
    std::uint64_t priority{};
    std::uint64_t flags{};
};

class RoomAssignmentStore {
public:
    bool upsert(RoomAssignment value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const RoomAssignment* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<RoomAssignment> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const RoomAssignment& value) noexcept;
    std::vector<RoomAssignment> records_;
};

} // namespace elysium::world
