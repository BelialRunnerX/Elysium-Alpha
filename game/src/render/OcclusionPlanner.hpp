// Intended function: Track conservative occlusion candidates and visibility decisions without making render state authoritative gameplay truth.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::render {
struct OcclusionCandidate {
    std::uint64_t objectId{};
    std::uint64_t boundsHash{};
    double distance{};
    double screenArea{};
    double confidence{};
    std::uint64_t flags{};
};
class OcclusionCandidateRegistry {
public:
    bool publish(OcclusionCandidate record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const OcclusionCandidate* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<OcclusionCandidate>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const OcclusionCandidate& r) noexcept;
    std::vector<OcclusionCandidate> records_;
};
} // namespace elysium::render
