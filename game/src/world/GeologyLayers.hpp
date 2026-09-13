// Intended function: Model deterministic planetary strata, local geology samples, and ore-bearing layer metadata for mining/worldgen.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct GeologyLayer {
    std::uint64_t layerId{};
    std::uint64_t materialId{};
    double minDepth{};
    double maxDepth{};
    double hardness{};
    std::uint64_t oreBias{};
};

class GeologyLayerStore {
public:
    bool upsert(GeologyLayer value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const GeologyLayer* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<GeologyLayer> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const GeologyLayer& value) noexcept;
    std::vector<GeologyLayer> records_;
};

} // namespace elysium::world
