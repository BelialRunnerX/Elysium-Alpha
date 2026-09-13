// Intended function: Represent action mappings, contexts, chord/modifier bindings, sensitivity, invert options, and device-independent input intents.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::input {
struct InputBinding {
    std::uint64_t bindingId{};
    std::uint64_t actionId{};
    std::uint64_t contextId{};
    std::uint64_t primaryCode{};
    std::uint64_t modifierMask{};
    std::uint64_t flags{};
};
class InputBindingStore {
public:
 bool put(InputBinding v); bool erase(std::uint64_t id);
 [[nodiscard]] const InputBinding* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<InputBinding>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const InputBinding& v) noexcept; std::vector<InputBinding> values_;
};
} // namespace elysium::input
