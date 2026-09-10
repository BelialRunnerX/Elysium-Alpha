#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent local save-slot metadata, compatibility, thumbnails, backup state, and user-facing selection without owning simulation serialization.
struct SaveSlotManagerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SaveSlotManagerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SaveSlotManagerService {
public:
    bool submit(const SaveSlotManagerCommand& command);
    const SaveSlotManagerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SaveSlotManagerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SaveSlotManagerRecord> records_;
};

}
