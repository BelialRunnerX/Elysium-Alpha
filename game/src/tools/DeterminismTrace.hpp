#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Record ordered deterministic decision facts and stable inputs for later divergence debugging.
struct DeterminismTraceCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct DeterminismTraceRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class DeterminismTraceService {
public:
    bool submit(const DeterminismTraceCommand& command);
    const DeterminismTraceRecord* lookup(std::uint64_t subjectId) const;
    std::vector<DeterminismTraceRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, DeterminismTraceRecord> records_;
};

}
