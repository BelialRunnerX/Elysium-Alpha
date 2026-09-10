// Intended function: Project save generations, schemas, chunk deltas, stable objects, and recovery state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct SaveInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveInspectorSystem {
public:
 bool submit(const SaveInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveInspectorRecord> snapshot() const;
 std::vector<SaveInspectorNotice> drainNotices(); void clear();
private:
 SaveInspectorRecord* mutableFind(std::uint64_t); void notice(const SaveInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveInspectorRecord> records_; std::vector<SaveInspectorNotice> notices_;
};
}
