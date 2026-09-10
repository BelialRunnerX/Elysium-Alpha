// Intended function: Persist bounded versioned script-owned state keyed by mod/script/stable object identity without arbitrary memory serialization.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::scripting {
struct ScriptStateRecord {
    std::uint64_t recordId{};
    std::uint64_t scriptId{};
    std::uint64_t ownerId{};
    std::uint64_t schema{};
    std::uint64_t payloadHash{};
    std::uint64_t revision{};
};
class ScriptStateRecordCollection {
public:
 bool store(ScriptStateRecord value); bool erase(std::uint64_t id); [[nodiscard]] const ScriptStateRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<ScriptStateRecord>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const ScriptStateRecord& v) noexcept; std::vector<ScriptStateRecord> rows_;
};
}
