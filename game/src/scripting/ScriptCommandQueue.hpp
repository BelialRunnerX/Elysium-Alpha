// Intended function: Queue bounded script-originated commands through the same validated stable command path as UI/AI rather than direct world mutation.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::scripting {
struct ScriptCommand {
    std::uint64_t commandId{};
    std::uint64_t scriptId{};
    std::uint64_t kind{};
    std::uint64_t targetId{};
    std::uint64_t payloadHash{};
    std::uint64_t sequence{};
};
class ScriptCommandCollection {
public:
 bool store(ScriptCommand value); bool erase(std::uint64_t id); [[nodiscard]] const ScriptCommand* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ScriptCommand>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ScriptCommand& v) noexcept; std::vector<ScriptCommand> rows_;
};
}
