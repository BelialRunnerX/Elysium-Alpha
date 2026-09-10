// Intended function: Map durable StableIds to transient active-shard handles while preventing runtime handles from becoming save/network identity.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ecs {
struct StableIdBinding {
    std::uint64_t stableId{};
    double runtimeHandle{};
    std::uint64_t typeId{};
    std::uint64_t shardId{};
    std::uint64_t revision{};
    std::uint64_t flags{};
};
class StableIdBindingCollection {
public:
 bool store(StableIdBinding value); bool erase(std::uint64_t id); [[nodiscard]] const StableIdBinding* find(std::uint64_t id) const; [[nodiscard]] const std::vector<StableIdBinding>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const StableIdBinding& v) noexcept; std::vector<StableIdBinding> rows_;
};
}
