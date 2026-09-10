// Intended function: Track composite fibers, matrices, cure cycles, defects, and structural-quality output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct CompositeLayupRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CompositeLayupRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CompositeLayupNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CompositeLayupSystem {
public:
 bool submit(const CompositeLayupRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CompositeLayupRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CompositeLayupRecord> snapshot() const;
 std::vector<CompositeLayupNotice> drainNotices(); void clear();
private:
 CompositeLayupRecord* mutableFind(std::uint64_t); void notice(const CompositeLayupRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CompositeLayupRecord> records_; std::vector<CompositeLayupNotice> notices_;
};
}
