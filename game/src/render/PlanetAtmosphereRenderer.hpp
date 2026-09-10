#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Provide backend-neutral atmosphere/scattering parameters derived from authoritative planet/environment summaries.
struct PlanetAtmosphereRendererCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct PlanetAtmosphereRendererRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class PlanetAtmosphereRendererService {
public:
    bool submit(const PlanetAtmosphereRendererCommand& command);
    const PlanetAtmosphereRendererRecord* lookup(std::uint64_t subjectId) const;
    std::vector<PlanetAtmosphereRendererRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, PlanetAtmosphereRendererRecord> records_;
};

}
