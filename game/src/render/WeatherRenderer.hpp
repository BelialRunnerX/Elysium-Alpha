#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Project rain, dust, snow, ash, lightning, fog, and storm intensity into renderer-neutral presentation commands.
struct WeatherRendererCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct WeatherRendererRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class WeatherRendererService {
public:
    bool submit(const WeatherRendererCommand& command);
    const WeatherRendererRecord* lookup(std::uint64_t subjectId) const;
    std::vector<WeatherRendererRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, WeatherRendererRecord> records_;
};

}
