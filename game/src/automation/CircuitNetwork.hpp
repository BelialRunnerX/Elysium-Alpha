#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Propagate bounded numeric and boolean signals across stable-ID automation links with deterministic ordering.
struct CircuitNetworkCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CircuitNetworkRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CircuitNetworkService {
public:
    bool submit(const CircuitNetworkCommand& command);
    const CircuitNetworkRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CircuitNetworkRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CircuitNetworkRecord> records_;
};

}
