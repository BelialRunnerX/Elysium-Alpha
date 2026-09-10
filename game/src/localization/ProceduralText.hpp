#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Assemble localized procedural names, descriptions, Chronicle entries, item lore, and mission text from stable tokens.
struct ProceduralTextCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ProceduralTextRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ProceduralTextService {
public:
    bool submit(const ProceduralTextCommand& command);
    const ProceduralTextRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ProceduralTextRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ProceduralTextRecord> records_;
};

}
