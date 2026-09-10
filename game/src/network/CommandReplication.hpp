// Intended function: Represent future client/server stable command envelopes with sequence, authority epoch, validation result, and reconciliation tick.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::network {
struct ReplicatedCommand {
    std::uint64_t commandId{};
    std::uint64_t actorId{};
    std::uint64_t sequence{};
    std::uint64_t authorityEpoch{};
    std::uint64_t tick{};
    std::uint64_t payloadHash{};
};
class ReplicatedCommandCollection {
public:
 bool store(ReplicatedCommand value); bool erase(std::uint64_t id); [[nodiscard]] const ReplicatedCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ReplicatedCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ReplicatedCommand& v) noexcept; std::vector<ReplicatedCommand> rows_;
};
}
