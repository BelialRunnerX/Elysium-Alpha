#pragma once

#include "render/GraphicsBackend.hpp"

#include <cstdint>
#include <unordered_map>

#include <raylib.h>

namespace elysium {

class RaylibGraphicsBackend final : public IGraphicsBackend {
public:
    RaylibGraphicsBackend() = default;
    ~RaylibGraphicsBackend() override;

    GraphicsMeshHandle uploadMesh(const CpuMeshData& data) override;
    void destroyMesh(GraphicsMeshHandle handle) override;
    GraphicsTextureHandle uploadTexture(const CpuTextureData& data) override;
    void destroyTexture(GraphicsTextureHandle handle) override;
    void drawMesh(GraphicsMeshHandle handle) const override;
    void drawMesh(GraphicsMeshHandle handle, const GraphicsDrawParams& params) const override;
    void drawMeshInstances(GraphicsMeshHandle handle,
                           std::span<const GraphicsInstance> instances,
                           Vec3 renderOrigin = {}) const override;
    GraphicsBackendCapabilities capabilities() const override { return {true, false}; }

private:
    std::uint32_t nextHandle_{1};
    std::uint32_t nextTextureHandle_{1};
    std::unordered_map<std::uint32_t,Model> models_;
    std::unordered_map<std::uint32_t,Texture2D> textures_;
};

} // namespace elysium
