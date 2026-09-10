// Intended function: Queue 16^3 microvoxel edits, damage masks, refinement ownership, and deterministic commit ordering.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::world {

struct MicroEdit {
    std::uint64_t editId{};
    std::uint64_t macroAddressKey{};
    std::uint64_t microIndex{};
    std::uint64_t materialId{};
    std::uint64_t operation{};
    std::uint64_t priority{};
};

class MicroEditStore {
public:
    bool upsert(MicroEdit value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const MicroEdit* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<MicroEdit> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const MicroEdit& value) noexcept;
    std::vector<MicroEdit> records_;
};

} // namespace elysium::world
