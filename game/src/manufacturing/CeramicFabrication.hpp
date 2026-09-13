// Intended function: Track advanced ceramic feedstock, sintering, defects, quality, and specialized output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct CeramicFabricationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CeramicFabricationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CeramicFabricationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CeramicFabricationSystem {
public:
 bool submit(const CeramicFabricationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CeramicFabricationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CeramicFabricationRecord> snapshot() const;
 std::vector<CeramicFabricationNotice> drainNotices(); void clear();
private:
 CeramicFabricationRecord* mutableFind(std::uint64_t); void notice(const CeramicFabricationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CeramicFabricationRecord> records_; std::vector<CeramicFabricationNotice> notices_;
};
}
