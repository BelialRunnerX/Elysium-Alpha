#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build market quotes, manifests, legality, ownership transfer, contracts, standing, and negotiation projections.
struct TradeScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct TradeScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class TradeScreenModelService {
public:
    bool submit(const TradeScreenModelCommand& command);
    const TradeScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<TradeScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, TradeScreenModelRecord> records_;
};

}
