// Intended function: Generate procedural Rift Horror body plans, abilities, vulnerabilities, phase rules, tells, and stable encounter identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::rift {
struct RiftHorrorDescriptor {
    std::uint64_t horrorId{};
    std::uint64_t grammarId{};
    std::uint64_t bodyHash{};
    std::uint64_t abilityHash{};
    std::uint64_t weaknessHash{};
    std::uint64_t tier{};
};
class RiftHorrorDescriptorStore {
public:
 bool put(RiftHorrorDescriptor v); bool erase(std::uint64_t id);
 [[nodiscard]] const RiftHorrorDescriptor* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<RiftHorrorDescriptor>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const RiftHorrorDescriptor& v) noexcept; std::vector<RiftHorrorDescriptor> values_;
};
} // namespace elysium::rift
