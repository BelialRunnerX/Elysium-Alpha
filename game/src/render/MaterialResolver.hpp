// Intended function: Resolve voxel/item/structure material identity into renderer-neutral surface parameters and texture/material handles.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct MaterialRenderState {
    std::uint64_t materialId{};
    std::uint64_t albedoId{};
    std::uint64_t normalId{};
    std::uint64_t emissiveId{};
    double roughness{};
    std::uint64_t flags{};
};
class MaterialRenderStateRegistry {
public:
    bool publish(MaterialRenderState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const MaterialRenderState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<MaterialRenderState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const MaterialRenderState& r) noexcept;
    std::vector<MaterialRenderState> records_;
};
} // namespace elysium::render
