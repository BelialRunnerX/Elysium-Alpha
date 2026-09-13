// Intended function: Track ship/vehicle fuel tanks, reserve policy, consumption rates, refuel intents, and stranded-state diagnostics.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct FuelState {
    std::uint64_t ownerId{};
    std::uint64_t fuelType{};
    std::uint64_t currentUnits{};
    std::uint64_t capacity{};
    std::uint64_t reserveUnits{};
    double burnRate{};
};
class FuelStateRegistry {
public:
    bool publish(FuelState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const FuelState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<FuelState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const FuelState& r) noexcept;
    std::vector<FuelState> records_;
};
} // namespace elysium::travel
