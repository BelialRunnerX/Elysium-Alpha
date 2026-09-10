// Intended function: Identify obsolete generations, unreferenced sidecars, and safe deletion candidates.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct SaveGarbageCollectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveGarbageCollectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveGarbageCollectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveGarbageCollectorSystem {
public:
 bool submit(const SaveGarbageCollectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveGarbageCollectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveGarbageCollectorRecord> snapshot() const;
 std::vector<SaveGarbageCollectorNotice> drainNotices(); void clear();
private:
 SaveGarbageCollectorRecord* mutableFind(std::uint64_t); void notice(const SaveGarbageCollectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveGarbageCollectorRecord> records_; std::vector<SaveGarbageCollectorNotice> notices_;
};
}
