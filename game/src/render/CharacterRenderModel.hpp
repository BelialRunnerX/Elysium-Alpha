#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build renderer-facing character equipment, pose, damage, status, and cosmetic snapshots without exposing ECS handles.
struct CharacterRenderModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct CharacterRenderModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class CharacterRenderModelService {
public:
    bool submit(const CharacterRenderModelCommand& command);
    const CharacterRenderModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<CharacterRenderModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, CharacterRenderModelRecord> records_;
};

}
