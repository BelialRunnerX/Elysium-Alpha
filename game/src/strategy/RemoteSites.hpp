// Intended function: Represent off-screen settlements/outposts as compact stable strategic summaries for behavioral LOD simulation.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::strategy {

struct RemoteSiteState {
    std::uint64_t siteId{};
    std::uint64_t population{};
    double industryScore{};
    double defenseScore{};
    double supplyScore{};
    double threatScore{};
};

class RemoteSiteStateStore {
public:
    bool upsert(RemoteSiteState value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const RemoteSiteState* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<RemoteSiteState> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const RemoteSiteState& value) noexcept;
    std::vector<RemoteSiteState> records_;
};

} // namespace elysium::strategy
