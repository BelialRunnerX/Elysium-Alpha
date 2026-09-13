// Intended function: Track lenses, crystal growth, coatings, calibration, and optical-quality output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct OpticsFabricationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct OpticsFabricationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct OpticsFabricationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class OpticsFabricationSystem {
public:
 bool submit(const OpticsFabricationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const OpticsFabricationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<OpticsFabricationRecord> snapshot() const;
 std::vector<OpticsFabricationNotice> drainNotices(); void clear();
private:
 OpticsFabricationRecord* mutableFind(std::uint64_t); void notice(const OpticsFabricationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<OpticsFabricationRecord> records_; std::vector<OpticsFabricationNotice> notices_;
};
}
