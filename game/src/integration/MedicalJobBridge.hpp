// Intended function: Translate triage/treatment plans into reservable fortress jobs with patient, bed, tool, medicine, and practitioner dependencies.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct MedicalJobIntent {
    std::uint64_t intentId{};
    std::uint64_t patientId{};
    std::uint64_t procedureId{};
    std::uint64_t practitionerId{};
    std::uint64_t priority{};
    std::uint64_t state{};
};
class MedicalJobIntentIndex {
public:
 bool upsert(MedicalJobIntent value); bool erase(std::uint64_t id); [[nodiscard]] const MedicalJobIntent* find(std::uint64_t id) const; [[nodiscard]] const std::vector<MedicalJobIntent>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const MedicalJobIntent& value) noexcept; std::vector<MedicalJobIntent> rows_;
};
}
