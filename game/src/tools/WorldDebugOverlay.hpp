#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build renderer-neutral debug primitives for addresses, chunks, nav, rooms, power, atmosphere, claims, jobs, and AI.
struct WorldDebugOverlayCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct WorldDebugOverlayRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class WorldDebugOverlayService {
public:
    bool submit(const WorldDebugOverlayCommand& command);
    const WorldDebugOverlayRecord* lookup(std::uint64_t subjectId) const;
    std::vector<WorldDebugOverlayRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, WorldDebugOverlayRecord> records_;
};

}
