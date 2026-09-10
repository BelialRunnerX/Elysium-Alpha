#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Score voxel and object cover from attack direction without converting world cells into ECS entities.
struct CoverSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CoverSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CoverSystemService {
public:
    bool submit(const CoverSystemCommand& command);
    const CoverSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CoverSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CoverSystemRecord> records_;
};

}
