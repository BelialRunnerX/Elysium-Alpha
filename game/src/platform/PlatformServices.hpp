// Intended function: Represent platform services such as clipboard, URL open, user paths, locale, input devices, and display capabilities.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::platform {
struct PlatformCapability {
    std::uint64_t capabilityId{};
    std::uint64_t kind{};
    double value{};
    std::uint64_t available{};
    std::uint64_t revision{};
    std::uint64_t flags{};
};
class PlatformCapabilityCollection {
public:
 bool store(PlatformCapability value); bool erase(std::uint64_t id); [[nodiscard]] const PlatformCapability* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PlatformCapability>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const PlatformCapability& v) noexcept; std::vector<PlatformCapability> rows_;
};
}
