// Intended function: Track mission objective dependency graphs, optional branches, failures, and completion.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::mission {
struct ObjectiveGraphRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ObjectiveGraphRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ObjectiveGraphNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ObjectiveGraphSystem {
public:
 bool submit(const ObjectiveGraphRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ObjectiveGraphRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ObjectiveGraphRecord> snapshot() const;
 std::vector<ObjectiveGraphNotice> drainNotices(); void clear();
private:
 ObjectiveGraphRecord* mutableFind(std::uint64_t); void notice(const ObjectiveGraphRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ObjectiveGraphRecord> records_; std::vector<ObjectiveGraphNotice> notices_;
};
}
