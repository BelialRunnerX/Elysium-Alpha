#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Store reusable base/ship construction templates using stable content IDs and relative placement records.
struct BlueprintLibraryCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct BlueprintLibraryRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class BlueprintLibraryService {
public:
    bool submit(const BlueprintLibraryCommand& command);
    const BlueprintLibraryRecord* lookup(std::uint64_t subjectId) const;
    std::vector<BlueprintLibraryRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, BlueprintLibraryRecord> records_;
};

}
