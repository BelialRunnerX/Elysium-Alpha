// Intended function: imported render implementation for FakeGraphicsBackend; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "render/GraphicsBackend.hpp"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace elysium {

// Contract probe for headless tests/tools. This does not implement culling or
// rendering policy; it only records the exact backend calls issued by renderer
// orchestration, which keeps the stub from becoming a counterfeit renderer.
class FakeGraphicsBackend final : public IGraphicsBackend {
public:
    enum class EventKind : std::uint8_t { Upload, Destroy, Draw, DrawInstances, UploadTexture, DestroyTexture };
    struct Event {
        EventKind kind{};
        GraphicsMeshHandle handle{};
        GraphicsDrawParams draw{};
        int vertices{};
        int triangles{};
        std::size_t bytes{};
        int instances{1};
    };

    GraphicsMeshHandle uploadMesh(const CpuMeshData& data) override {
        if (data.empty()) return {};
        const GraphicsMeshHandle handle{next_++};
        live_.emplace(handle.value,Live{data.vertexCount(),data.triangleCount(),data.estimatedBytes()});
        ++uploads;
        events_.push_back({EventKind::Upload,handle,{},data.vertexCount(),data.triangleCount(),data.estimatedBytes()});
        return handle;
    }

    void destroyMesh(GraphicsMeshHandle handle) override {
        if (!handle) return;
        const auto it=live_.find(handle.value);
        if (it==live_.end()) return;
        events_.push_back({EventKind::Destroy,handle,{},it->second.vertices,it->second.triangles,it->second.bytes});
        live_.erase(it);
        ++destroys;
    }

    GraphicsTextureHandle uploadTexture(const CpuTextureData& data) override {
        if (!data.valid()) return {};
        const GraphicsTextureHandle handle{nextTexture_++};
        textures_.emplace(handle.value,data.estimatedBytes());
        Event event{}; event.kind=EventKind::UploadTexture; event.handle={handle.value}; event.bytes=data.estimatedBytes(); event.instances=0;
        events_.push_back(event);
        ++textureUploads;
        return handle;
    }

    void destroyTexture(GraphicsTextureHandle handle) override {
        if (!handle) return;
        const auto it=textures_.find(handle.value);
        if (it==textures_.end()) return;
        Event event{}; event.kind=EventKind::DestroyTexture; event.handle={handle.value}; event.bytes=it->second; event.instances=0;
        events_.push_back(event);
        textures_.erase(it);
        ++textureDestroys;
    }

    void drawMesh(GraphicsMeshHandle handle) const override {
        drawMesh(handle,{});
    }

    void drawMesh(GraphicsMeshHandle handle, const GraphicsDrawParams& params) const override {
        if (!handle) return;
        const auto it=live_.find(handle.value);
        if (it==live_.end()) return;
        events_.push_back({EventKind::Draw,handle,params,it->second.vertices,it->second.triangles,it->second.bytes});
        ++draws;
    }


    void drawMeshInstances(GraphicsMeshHandle handle, std::span<const GraphicsInstance> instances, Vec3 renderOrigin = {}) const override {
        if (!handle || instances.empty()) return;
        const auto it=live_.find(handle.value);
        if (it==live_.end()) return;
        GraphicsDrawParams first{};
        first.translation=instances.front().translation-renderOrigin;
        first.uniformScale=instances.front().uniformScale;
        events_.push_back({EventKind::DrawInstances,handle,first,it->second.vertices,it->second.triangles,it->second.bytes,static_cast<int>(instances.size())});
        draws+=static_cast<int>(instances.size());
        instanceDraws+=static_cast<int>(instances.size());
    }

    mutable int draws{};
    mutable int instanceDraws{};
    int uploads{};
    int destroys{};
    int textureUploads{};
    int textureDestroys{};

    std::size_t liveCount() const { return live_.size(); }
    std::size_t liveTextureCount() const { return textures_.size(); }
    std::size_t liveBytes() const {
        std::size_t bytes=0;
        for (const auto& [_,v]:live_) bytes+=v.bytes;
        for (const auto& [_,v]:textures_) bytes+=v;
        return bytes;
    }
    const std::vector<Event>& events() const { return events_; }
    void clearEvents() const { events_.clear(); }
    int count(EventKind kind) const {
        int n=0;
        for (const auto& e:events_) if (e.kind==kind) ++n;
        return n;
    }

private:
    struct Live { int vertices{}; int triangles{}; std::size_t bytes{}; };
    std::uint32_t next_{1};
    std::uint32_t nextTexture_{1};
    std::unordered_map<std::uint32_t,Live> live_;
    std::unordered_map<std::uint32_t,std::size_t> textures_;
    mutable std::vector<Event> events_;
};

} // namespace elysium
