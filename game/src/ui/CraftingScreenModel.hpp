#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build recipe availability, requirements, queues, blockers, and crafting command intents.
struct CraftingScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CraftingScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CraftingScreenModelService {
public:
    bool submit(const CraftingScreenModelCommand& command);
    const CraftingScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CraftingScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CraftingScreenModelRecord> records_;
};

}
