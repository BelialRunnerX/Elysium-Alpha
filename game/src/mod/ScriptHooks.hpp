// Intended function: Expose bounded event-hook declarations for future scripting while keeping world mutation behind validated command APIs.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::mod {
struct ScriptHookRecord {
    std::uint64_t hookId{};
    std::uint64_t eventType{};
    std::uint64_t scriptId{};
    std::uint64_t priority{};
    std::uint64_t permissionMask{};
    std::uint64_t flags{};
};
class ScriptHookRecordRegistry {
public:
    bool publish(ScriptHookRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ScriptHookRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ScriptHookRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ScriptHookRecord& r) noexcept;
    std::vector<ScriptHookRecord> records_;
};
} // namespace elysium::mod
