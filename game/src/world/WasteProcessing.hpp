// Intended function: Represent waste streams, contamination classes, recycling queues, and safe disposal/reclamation outputs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct WasteBatch {
    std::uint64_t stableId{};
    std::uint64_t wasteType{};
    std::uint64_t units{};
    std::uint64_t contamination{};
    double recoveryPotential{};
    std::uint64_t state{};
};

class WasteBatchStore {
public:
    bool upsert(WasteBatch value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const WasteBatch* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<WasteBatch> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const WasteBatch& value) noexcept;
    std::vector<WasteBatch> records_;
};

} // namespace elysium::world
