// Intended function: Aggregate local sensor nodes into bounded contact reports used by defense, automation, alerts, and exploration.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct SensorContact {
    std::uint64_t contactId{};
    std::uint64_t sourceNodeId{};
    std::uint64_t category{};
    double strength{};
    double confidence{};
    std::uint64_t ageTicks{};
};

class SensorContactStore {
public:
    bool upsert(SensorContact value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const SensorContact* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<SensorContact> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const SensorContact& value) noexcept;
    std::vector<SensorContact> records_;
};

} // namespace elysium::world
