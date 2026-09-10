// Intended function: Capture crash/session metadata, build identity, recent log/event hashes, save slot, and optional diagnostic attachments.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::platform {
struct CrashRecord {
    std::uint64_t crashId{};
    std::uint64_t buildId{};
    std::uint64_t sessionId{};
    std::uint64_t tick{};
    std::uint64_t logHash{};
    std::uint64_t flags{};
};
class CrashRecordCollection {
public:
 bool store(CrashRecord value); bool erase(std::uint64_t id); [[nodiscard]] const CrashRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<CrashRecord>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const CrashRecord& v) noexcept; std::vector<CrashRecord> rows_;
};
}
