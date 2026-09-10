#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track water, food, housing, medicine, recreation, security, education, and communications service coverage.
struct SettlementServicesCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SettlementServicesRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SettlementServicesService {
public:
    bool submit(const SettlementServicesCommand& command);
    const SettlementServicesRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SettlementServicesRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SettlementServicesRecord> records_;
};

}
