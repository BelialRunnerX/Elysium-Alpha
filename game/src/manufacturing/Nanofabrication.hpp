// Intended function: Track nanomaterial feedstock, patterning, assembly, contamination, and precision output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct NanofabricationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct NanofabricationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct NanofabricationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class NanofabricationSystem {
public:
 bool submit(const NanofabricationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const NanofabricationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<NanofabricationRecord> snapshot() const;
 std::vector<NanofabricationNotice> drainNotices(); void clear();
private:
 NanofabricationRecord* mutableFind(std::uint64_t); void notice(const NanofabricationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<NanofabricationRecord> records_; std::vector<NanofabricationNotice> notices_;
};
}
