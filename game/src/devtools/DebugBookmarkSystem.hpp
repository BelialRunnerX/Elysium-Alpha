// Intended function: Store stable debug bookmarks for entities, sites, chunks, events, and time ranges.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct DebugBookmarkSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct DebugBookmarkSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct DebugBookmarkSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class DebugBookmarkSystemSystem {
public:
 bool submit(const DebugBookmarkSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const DebugBookmarkSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<DebugBookmarkSystemRecord> snapshot() const;
 std::vector<DebugBookmarkSystemNotice> drainNotices(); void clear();
private:
 DebugBookmarkSystemRecord* mutableFind(std::uint64_t); void notice(const DebugBookmarkSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<DebugBookmarkSystemRecord> records_; std::vector<DebugBookmarkSystemNotice> notices_;
};
}
