// Intended function: Select stable combat targets using threat, visibility, range, faction, cover, and deterministic tie-breaks.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::combat {

struct TargetCandidate {
    std::uint64_t stableId{};
    double threat{};
    double distance{};
    double visibility{};
    double cover{};
    std::uint64_t priority{};
};

class TargetCandidateStore {
public:
    bool upsert(TargetCandidate value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const TargetCandidate* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<TargetCandidate> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const TargetCandidate& value) noexcept;
    std::vector<TargetCandidate> records_;
};

} // namespace elysium::combat
