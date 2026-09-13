#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build renderer-facing ship hull/module/damage/thruster/shield snapshots keyed by durable ship identity.
struct ShipRenderModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ShipRenderModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ShipRenderModelService {
public:
    bool submit(const ShipRenderModelCommand& command);
    const ShipRenderModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ShipRenderModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ShipRenderModelRecord> records_;
};

}
