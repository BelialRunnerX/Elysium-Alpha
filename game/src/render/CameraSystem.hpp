// Intended function: Track camera pose, local gravity frame, third/first person mode, zoom, smoothing, collision, floating origin, and transitions.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::render {
struct CameraState {
    std::uint64_t cameraId{};
    std::uint64_t targetId{};
    std::uint64_t positionHash{};
    std::uint64_t orientationHash{};
    double fov{};
    std::uint64_t mode{};
};
class CameraStateCollection {
public:
 bool store(CameraState value); bool erase(std::uint64_t id); [[nodiscard]] const CameraState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CameraState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CameraState& v) noexcept; std::vector<CameraState> rows_;
};
}
