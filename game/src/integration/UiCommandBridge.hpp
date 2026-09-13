// Intended function: Translate validated UI command models into owner-system command requests while keeping UI presentation-only.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::integration {
struct UiCommandDispatch {
    std::uint64_t dispatchId{};
    std::uint64_t commandId{};
    std::uint64_t ownerSystem{};
    std::uint64_t targetId{};
    std::uint64_t sequence{};
    std::uint64_t state{};
};
class UiCommandDispatchIndex {
public:
 bool upsert(UiCommandDispatch value); bool erase(std::uint64_t id); [[nodiscard]] const UiCommandDispatch* find(std::uint64_t id) const; [[nodiscard]] const std::vector<UiCommandDispatch>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const UiCommandDispatch& value) noexcept; std::vector<UiCommandDispatch> rows_;
};
}
