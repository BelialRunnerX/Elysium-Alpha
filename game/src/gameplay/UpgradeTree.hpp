#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent data-driven technology and facility upgrade branches with stable unlock identities and prerequisites.
struct UpgradeTreeCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct UpgradeTreeRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class UpgradeTreeService {
public:
    bool submit(const UpgradeTreeCommand& command);
    const UpgradeTreeRecord* lookup(std::uint64_t subjectId) const;
    std::vector<UpgradeTreeRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, UpgradeTreeRecord> records_;
};

}
