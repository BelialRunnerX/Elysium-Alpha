#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent boarding breaches, compartments, teams, objectives, defenders, capture state, and retreat routes.
struct BoardingSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct BoardingSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class BoardingSystemService {
public:
    bool submit(const BoardingSystemCommand& command);
    const BoardingSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<BoardingSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, BoardingSystemRecord> records_;
};

}
