#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register deterministic save-schema migration steps and reject ambiguous or unsupported upgrade paths.
struct SchemaMigrationRegistryRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct SchemaMigrationRegistryState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class SchemaMigrationRegistrySystem {
public:
    bool apply(const SchemaMigrationRegistryRequest& request);
    bool erase(std::uint64_t targetId);
    const SchemaMigrationRegistryState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, SchemaMigrationRegistryState> states_;
};

} // namespace elysium
