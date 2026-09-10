// Intended function: Project chunk addresses, ownership, generator versions, edits, and spatial state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct WorldInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct WorldInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct WorldInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class WorldInspectorSystem {
public:
 bool submit(const WorldInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const WorldInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<WorldInspectorRecord> snapshot() const;
 std::vector<WorldInspectorNotice> drainNotices(); void clear();
private:
 WorldInspectorRecord* mutableFind(std::uint64_t); void notice(const WorldInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<WorldInspectorRecord> records_; std::vector<WorldInspectorNotice> notices_;
};
}
