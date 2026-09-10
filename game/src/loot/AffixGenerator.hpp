#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate deterministic Diablo-style prefixes, suffixes, tiers, rolls, and incompatibility groups from item seeds.
struct AffixGeneratorCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct AffixGeneratorRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class AffixGeneratorService {
public:
    bool submit(const AffixGeneratorCommand& command);
    const AffixGeneratorRecord* lookup(std::uint64_t subjectId) const;
    std::vector<AffixGeneratorRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, AffixGeneratorRecord> records_;
};

}
