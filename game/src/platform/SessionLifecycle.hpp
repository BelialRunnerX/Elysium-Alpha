#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Coordinate new-game, load, active-session, suspend, return-to-menu, and shutdown transition intents.
struct SessionLifecycleCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SessionLifecycleRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SessionLifecycleService {
public:
    bool submit(const SessionLifecycleCommand& command);
    const SessionLifecycleRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SessionLifecycleRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SessionLifecycleRecord> records_;
};

}
