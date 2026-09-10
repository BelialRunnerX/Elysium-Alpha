#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Queue authenticated remote machine control intents through stable object identities and authority checks.
struct RemoteControlCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct RemoteControlRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class RemoteControlService {
public:
    bool submit(const RemoteControlCommand& command);
    const RemoteControlRecord* lookup(std::uint64_t subjectId) const;
    std::vector<RemoteControlRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, RemoteControlRecord> records_;
};

}
