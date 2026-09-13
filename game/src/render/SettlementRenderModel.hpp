#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build renderer-facing settlement district, utility, alert, construction, and population summaries.
struct SettlementRenderModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SettlementRenderModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SettlementRenderModelService {
public:
    bool submit(const SettlementRenderModelCommand& command);
    const SettlementRenderModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SettlementRenderModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SettlementRenderModelRecord> records_;
};

}
