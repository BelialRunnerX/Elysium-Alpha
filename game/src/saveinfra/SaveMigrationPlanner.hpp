// Intended function: Build ordered schema migration paths for core, world, mod, and subsystem state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct SaveMigrationPlannerRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveMigrationPlannerRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveMigrationPlannerNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveMigrationPlannerSystem {
public:
 bool submit(const SaveMigrationPlannerRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveMigrationPlannerRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveMigrationPlannerRecord> snapshot() const;
 std::vector<SaveMigrationPlannerNotice> drainNotices(); void clear();
private:
 SaveMigrationPlannerRecord* mutableFind(std::uint64_t); void notice(const SaveMigrationPlannerRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveMigrationPlannerRecord> records_; std::vector<SaveMigrationPlannerNotice> notices_;
};
}
