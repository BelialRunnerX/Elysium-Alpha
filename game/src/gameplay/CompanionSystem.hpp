#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track recruited companions, loyalty, role, equipment, orders, relationship hooks, and active/remote representation.
struct CompanionSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CompanionSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CompanionSystemService {
public:
    bool submit(const CompanionSystemCommand& command);
    const CompanionSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CompanionSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CompanionSystemRecord> records_;
};

}
