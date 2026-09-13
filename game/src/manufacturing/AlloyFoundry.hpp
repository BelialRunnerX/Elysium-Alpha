// Intended function: Track alloy batches, furnace recipes, impurities, quality, thermal state, and output provenance.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct AlloyFoundryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AlloyFoundryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AlloyFoundryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AlloyFoundrySystem {
public:
 bool submit(const AlloyFoundryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AlloyFoundryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AlloyFoundryRecord> snapshot() const;
 std::vector<AlloyFoundryNotice> drainNotices(); void clear();
private:
 AlloyFoundryRecord* mutableFind(std::uint64_t); void notice(const AlloyFoundryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AlloyFoundryRecord> records_; std::vector<AlloyFoundryNotice> notices_;
};
}
