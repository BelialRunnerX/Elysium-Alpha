#include "render/RaylibGraphicsBackend.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

namespace elysium {
namespace {
Color toRaylib(GraphicsColor c) { return Color{c.r,c.g,c.b,c.a}; }
}

RaylibGraphicsBackend::~RaylibGraphicsBackend() {
    std::vector<std::uint32_t> meshHandles;
    meshHandles.reserve(models_.size());
    for (const auto& [id,_] : models_) meshHandles.push_back(id);
    for (const auto id : meshHandles) destroyMesh(GraphicsMeshHandle{id});

    std::vector<std::uint32_t> textureHandles;
    textureHandles.reserve(textures_.size());
    for (const auto& [id,_] : textures_) textureHandles.push_back(id);
    for (const auto id : textureHandles) destroyTexture(GraphicsTextureHandle{id});
}

GraphicsMeshHandle RaylibGraphicsBackend::uploadMesh(const CpuMeshData& data) {
    if (data.empty()) return {};
    Mesh mesh{};
    mesh.vertexCount = data.vertexCount();
    mesh.triangleCount = data.triangleCount();
    mesh.vertices = static_cast<float*>(MemAlloc(sizeof(float) * data.vertices.size()));
    mesh.normals = static_cast<float*>(MemAlloc(sizeof(float) * data.normals.size()));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(sizeof(unsigned char) * data.colors.size()));
    std::memcpy(mesh.vertices,data.vertices.data(),sizeof(float)*data.vertices.size());
    std::memcpy(mesh.normals,data.normals.data(),sizeof(float)*data.normals.size());
    std::memcpy(mesh.colors,data.colors.data(),sizeof(unsigned char)*data.colors.size());
    UploadMesh(&mesh,false);
    Model model=LoadModelFromMesh(mesh);
    const std::uint32_t id=nextHandle_++;
    models_.emplace(id,model);
    return GraphicsMeshHandle{id};
}

void RaylibGraphicsBackend::destroyMesh(GraphicsMeshHandle handle) {
    if (!handle) return;
    const auto it=models_.find(handle.value);
    if(it==models_.end()) return;
    UnloadModel(it->second);
    models_.erase(it);
}

GraphicsTextureHandle RaylibGraphicsBackend::uploadTexture(const CpuTextureData& data) {
    if (!data.valid()) return {};
    std::vector<unsigned char> rgba(static_cast<std::size_t>(data.width) * static_cast<std::size_t>(data.height) * 4U);
    for (std::size_t i=0, p=0; i<rgba.size(); i+=4, p+=static_cast<std::size_t>(data.channels)) {
        const auto remaining = data.pixels.size() - std::min(p, data.pixels.size());
        const unsigned char r = remaining > 0 ? data.pixels[p] : 255;
        const unsigned char g = data.channels > 1 && remaining > 1 ? data.pixels[p+1] : r;
        const unsigned char b = data.channels > 2 && remaining > 2 ? data.pixels[p+2] : r;
        const unsigned char a = data.channels > 3 && remaining > 3 ? data.pixels[p+3] : 255;
        rgba[i]=r; rgba[i+1]=g; rgba[i+2]=b; rgba[i+3]=a;
    }
    Image image{};
    image.data = rgba.data();
    image.width = data.width;
    image.height = data.height;
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    Texture2D texture = LoadTextureFromImage(image);
    if (texture.id == 0) return {};
    const std::uint32_t id=nextTextureHandle_++;
    textures_.emplace(id,texture);
    return GraphicsTextureHandle{id};
}

void RaylibGraphicsBackend::destroyTexture(GraphicsTextureHandle handle) {
    if (!handle) return;
    const auto it=textures_.find(handle.value);
    if (it==textures_.end()) return;
    UnloadTexture(it->second);
    textures_.erase(it);
}

void RaylibGraphicsBackend::drawMesh(GraphicsMeshHandle handle) const {
    drawMesh(handle,{});
}

void RaylibGraphicsBackend::drawMesh(GraphicsMeshHandle handle, const GraphicsDrawParams& params) const {
    if (!handle) return;
    const auto it=models_.find(handle.value);
    if(it==models_.end()) return;
    DrawModel(it->second,Vector3{params.translation.x,params.translation.y,params.translation.z},
              params.uniformScale,toRaylib(params.tint));
}

void RaylibGraphicsBackend::drawMeshInstances(GraphicsMeshHandle handle,
                                              std::span<const GraphicsInstance> instances,
                                              Vec3 renderOrigin) const {
    // Correct fallback first; hardware instancing can replace this without
    // changing orchestration or stable asset identities.
    for (const auto& instance : instances) {
        GraphicsDrawParams p{};
        p.translation=instance.translation-renderOrigin;
        p.uniformScale=instance.uniformScale;
        p.tint=instance.tint;
        drawMesh(handle,p);
    }
}

} // namespace elysium
