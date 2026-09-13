// Intended function: Schedule conveyors, loaders, drones, cargo rail, and ship transfers using stable endpoints and bounded queues.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::automation {
struct FreightOrder {
    std::uint64_t orderId{};
    std::uint64_t sourceId{};
    std::uint64_t destinationId{};
    std::uint64_t itemId{};
    std::uint64_t units{};
    std::uint64_t priority{};
};
class FreightOrderRegistry {
public:
    bool publish(FreightOrder record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const FreightOrder* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<FreightOrder>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const FreightOrder& r) noexcept;
    std::vector<FreightOrder> records_;
};
} // namespace elysium::automation
