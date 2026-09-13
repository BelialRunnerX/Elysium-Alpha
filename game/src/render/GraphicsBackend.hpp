#pragma once

#include "core/Math.hpp"
#include "world/VoxelMesher.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace elysium {

// Backend-local renderer identities. They are deliberately transient and must
// never be serialized or confused with stable world/ECS asset identities.
struct GraphicsMeshHandle {
    std::uint32_t value{};
    friend bool operator==(GraphicsMeshHandle, GraphicsMeshHandle) = default;
    explicit operator bool() const { return value != 0; }
};
struct GraphicsTextureHandle {
    std::uint32_t value{};
    friend bool operator==(GraphicsTextureHandle, GraphicsTextureHandle) = default;
    explicit operator bool() const { return value != 0; }
};

struct GraphicsColor {
    std::uint8_t r{255}, g{255}, b{255}, a{255};
};

// Canonical CPU-side RGBA texture payload. 1/3 channel payloads remain valid
// for tooling, but production backends may normalize them before upload.
struct CpuTextureData {
    int width{};
    int height{};
    int channels{4};
    std::vector<std::uint8_t> pixels;

    bool valid() const {
        return width > 0 && height > 0 && channels > 0 &&
               pixels.size() >= static_cast<std::size_t>(width) * static_cast<std::size_t>(height) *
                                static_cast<std::size_t>(channels);
    }
    std::size_t estimatedBytes() const { return pixels.size(); }
};

struct GraphicsDrawParams {
    Vec3 translation{};
    float uniformScale{1.0f};
    GraphicsColor tint{};
};

struct GraphicsInstance {
    Vec3 translation{};
    float uniformScale{1.0f};
    GraphicsColor tint{};
};

struct GraphicsBackendCapabilities {
    bool textures{};
    bool instancing{};
};

class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;

    // Called only on the graphics owner thread. Worker jobs may build CPU data
    // but must never mutate backend resources.
    virtual GraphicsMeshHandle uploadMesh(const CpuMeshData& data) = 0;
    virtual void destroyMesh(GraphicsMeshHandle handle) = 0;
    virtual void drawMesh(GraphicsMeshHandle handle) const = 0;

    // Extended production contract. Defaults preserve compatibility with
    // conservative/headless backends; concrete clients can accelerate them.
    virtual GraphicsTextureHandle uploadTexture(const CpuTextureData&) { return {}; }
    virtual void destroyTexture(GraphicsTextureHandle) {}
    virtual void drawMesh(GraphicsMeshHandle handle, const GraphicsDrawParams&) const { drawMesh(handle); }
    virtual void drawMeshInstances(GraphicsMeshHandle handle,
                                   std::span<const GraphicsInstance> instances,
                                   Vec3 renderOrigin = {}) const {
        for (const auto& instance : instances) {
            GraphicsDrawParams p{};
            p.translation = instance.translation - renderOrigin;
            p.uniformScale = instance.uniformScale;
            p.tint = instance.tint;
            drawMesh(handle, p);
        }
    }
    virtual GraphicsBackendCapabilities capabilities() const { return {}; }
};

} // namespace elysium
