// Intended function: Route bounded stable-address control messages between machines, sensors, doors, logistics, and defenses.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::automation {
struct ControlMessage {
    std::uint64_t messageId{};
    std::uint64_t sourceId{};
    std::uint64_t targetId{};
    std::uint64_t signalId{};
    double value{};
    std::uint64_t tick{};
};
class ControlMessageRegistry {
public:
    bool publish(ControlMessage record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ControlMessage* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ControlMessage>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ControlMessage& r) noexcept;
    std::vector<ControlMessage> records_;
};
} // namespace elysium::automation
