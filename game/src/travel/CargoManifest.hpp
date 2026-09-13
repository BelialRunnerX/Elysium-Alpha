// Intended function: Represent stable cargo manifests with mass, volume, ownership, hazard classification, and sealing requirements.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::travel {
struct CargoManifestState {
    std::uint64_t manifestId{};
    std::uint64_t ownerId{};
    double mass{};
    double volume{};
    double hazardClass{};
    std::uint64_t flags{};
};
class CargoManifestStateRegistry {
public:
    bool publish(CargoManifestState record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const CargoManifestState* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<CargoManifestState>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const CargoManifestState& r) noexcept;
    std::vector<CargoManifestState> records_;
};
} // namespace elysium::travel
