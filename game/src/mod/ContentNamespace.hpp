#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Allocate deterministic content namespaces and stable identifiers for base-game and mod-provided definitions.
struct ContentNamespaceRequest {
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double magnitude{0.0};
    std::uint32_t flags{0};
};

struct ContentNamespaceState {
    std::uint64_t sequence{0};
    std::uint64_t actorId{0};
    std::uint64_t targetId{0};
    double value{0.0};
    bool active{false};
};

class ContentNamespaceSystem {
public:
    bool apply(const ContentNamespaceRequest& request);
    bool erase(std::uint64_t targetId);
    const ContentNamespaceState* find(std::uint64_t targetId) const;
    std::vector<std::uint64_t> activeIds() const;
    void clear();

private:
    std::uint64_t nextSequence_{1};
    std::unordered_map<std::uint64_t, ContentNamespaceState> states_;
};

} // namespace elysium
