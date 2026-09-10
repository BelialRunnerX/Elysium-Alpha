// Intended function: Project stable entity/component summaries for debugging without granting mutation authority.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct EntityInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct EntityInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct EntityInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class EntityInspectorSystem {
public:
 bool submit(const EntityInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const EntityInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<EntityInspectorRecord> snapshot() const;
 std::vector<EntityInspectorNotice> drainNotices(); void clear();
private:
 EntityInspectorRecord* mutableFind(std::uint64_t); void notice(const EntityInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<EntityInspectorRecord> records_; std::vector<EntityInspectorNotice> notices_;
};
}
