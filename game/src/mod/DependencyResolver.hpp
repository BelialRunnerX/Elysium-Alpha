#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve mod dependency graphs, version constraints, optional dependencies, and deterministic load order.
struct DependencyResolverRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct DependencyResolverState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class DependencyResolverSystem {
public:
    bool apply(const DependencyResolverRequest& request);
    bool erase(std::uint64_t targetId);
    const DependencyResolverState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, DependencyResolverState> states_;
};

} // namespace elysium
