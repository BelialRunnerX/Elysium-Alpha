// Intended function: Describe deterministic cave-carving nodes and tunnel links for chunk-local generation and later meshing.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct CaveNode {
    std::uint64_t stableId{};
    double radius{};
    std::uint64_t branchCount{};
    double depth{};
    double wetness{};
    double hazard{};
};

class CaveNodeStore {
public:
    bool upsert(CaveNode value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const CaveNode* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<CaveNode> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const CaveNode& value) noexcept;
    std::vector<CaveNode> records_;
};

} // namespace elysium::world
