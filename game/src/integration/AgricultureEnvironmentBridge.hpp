// Intended function: Feed authoritative pressure, oxygen, temperature, water, light, and contamination samples into crop/livestock simulation adapters.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct AgricultureEnvironmentSample {
    std::uint64_t sampleId{};
    std::uint64_t siteId{};
    std::uint64_t pressure{};
    double oxygen{};
    double temperature{};
    std::uint64_t contamination{};
};
class AgricultureEnvironmentSampleIndex {
public:
 bool upsert(AgricultureEnvironmentSample value); bool erase(std::uint64_t id); [[nodiscard]] const AgricultureEnvironmentSample* find(std::uint64_t id) const; [[nodiscard]] const std::vector<AgricultureEnvironmentSample>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const AgricultureEnvironmentSample& value) noexcept; std::vector<AgricultureEnvironmentSample> rows_;
};
}
