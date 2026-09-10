#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Select deterministic contextual citizen, companion, squad, trader, and enemy bark intents without owning voice assets.
struct VoiceBarkSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct VoiceBarkSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class VoiceBarkSystemService {
public:
    bool submit(const VoiceBarkSystemCommand& command);
    const VoiceBarkSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<VoiceBarkSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, VoiceBarkSystemRecord> records_;
};

}
