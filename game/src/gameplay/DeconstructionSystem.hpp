#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Convert structures into bounded salvage outputs while preserving destruction and ownership history.
struct DeconstructionSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct DeconstructionSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class DeconstructionSystemService {
public:
    bool submit(const DeconstructionSystemCommand& command);
    const DeconstructionSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<DeconstructionSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, DeconstructionSystemRecord> records_;
};

}
