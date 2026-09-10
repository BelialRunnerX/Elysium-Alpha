// Intended function: Represent renderer-neutral player command intents for construction, logistics, machines, squads, travel, trade, and governance.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::ui {
struct CommandModel {
    std::uint64_t commandId{};
    std::uint64_t kind{};
    std::uint64_t targetId{};
    std::uint64_t secondaryId{};
    double value{};
    std::uint64_t flags{};
};
class CommandModelRegistry {
public:
    bool publish(CommandModel record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const CommandModel* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<CommandModel>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const CommandModel& r) noexcept;
    std::vector<CommandModel> records_;
};
} // namespace elysium::ui
