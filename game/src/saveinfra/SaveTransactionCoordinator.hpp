// Intended function: Track save publication stages, generation numbers, checksums, fallback, and commit barriers.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct SaveTransactionCoordinatorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveTransactionCoordinatorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveTransactionCoordinatorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveTransactionCoordinatorSystem {
public:
 bool submit(const SaveTransactionCoordinatorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveTransactionCoordinatorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveTransactionCoordinatorRecord> snapshot() const;
 std::vector<SaveTransactionCoordinatorNotice> drainNotices(); void clear();
private:
 SaveTransactionCoordinatorRecord* mutableFind(std::uint64_t); void notice(const SaveTransactionCoordinatorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveTransactionCoordinatorRecord> records_; std::vector<SaveTransactionCoordinatorNotice> notices_;
};
}
