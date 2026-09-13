// Intended function: Model stable fluid/atmosphere pipe segments and deterministic local flow requests between bounded endpoints.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct PipeSegment {
    std::uint64_t stableId{};
    std::uint64_t networkId{};
    double capacity{};
    double pressure{};
    double flow{};
    std::uint64_t flags{};
};

class PipeSegmentStore {
public:
    bool upsert(PipeSegment value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const PipeSegment* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<PipeSegment> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const PipeSegment& value) noexcept;
    std::vector<PipeSegment> records_;
};

} // namespace elysium::world
