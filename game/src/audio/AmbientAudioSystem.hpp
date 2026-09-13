#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate biome, weather, machine, settlement, ship, wildlife, and anomaly ambience requests.
struct AmbientAudioSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct AmbientAudioSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class AmbientAudioSystemService {
public:
    bool submit(const AmbientAudioSystemCommand& command);
    const AmbientAudioSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<AmbientAudioSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, AmbientAudioSystemRecord> records_;
};

}
