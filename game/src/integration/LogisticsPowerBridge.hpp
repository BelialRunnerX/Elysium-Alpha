// Intended function: Gate machine/logistics transfer and processing intents on bounded local power availability without consuming inputs on brownout.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct PoweredTransferIntent {
    std::uint64_t intentId{};
    std::uint64_t networkId{};
    std::uint64_t machineId{};
    double powerRequired{};
    std::uint64_t units{};
    std::uint64_t state{};
};
class PoweredTransferIntentIndex {
public:
 bool upsert(PoweredTransferIntent value); bool erase(std::uint64_t id); [[nodiscard]] const PoweredTransferIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<PoweredTransferIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const PoweredTransferIntent& value) noexcept; std::vector<PoweredTransferIntent> rows_;
};
}
