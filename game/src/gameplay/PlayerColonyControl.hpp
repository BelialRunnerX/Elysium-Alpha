#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent player settlement policy toggles, priorities, permissions, alerts, and strategic directives.
struct PlayerColonyControlCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct PlayerColonyControlRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class PlayerColonyControlService {
public:
    bool submit(const PlayerColonyControlCommand& command);
    const PlayerColonyControlRecord* lookup(std::uint64_t subjectId) const;
    std::vector<PlayerColonyControlRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, PlayerColonyControlRecord> records_;
};

}
