#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Queue player and workshop crafting operations with reserved inputs, progress, output ownership, and cancellation.
struct CraftingQueueCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CraftingQueueRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CraftingQueueService {
public:
    bool submit(const CraftingQueueCommand& command);
    const CraftingQueueRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CraftingQueueRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CraftingQueueRecord> records_;
};

}
