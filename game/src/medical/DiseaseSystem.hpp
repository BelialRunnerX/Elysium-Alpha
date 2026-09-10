// Intended function: Track infectious disease exposure, incubation, symptoms, immunity, transmission pressure, and quarantine policy.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::medical {
struct DiseaseState {
    std::uint64_t patientId{};
    std::uint64_t diseaseId{};
    double exposure{};
    double severity{};
    double immunity{};
    std::uint64_t state{};
};
class DiseaseStateStore {
public:
 bool put(DiseaseState v); bool erase(std::uint64_t id);
 [[nodiscard]] const DiseaseState* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<DiseaseState>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const DiseaseState& v) noexcept; std::vector<DiseaseState> values_;
};
} // namespace elysium::medical
