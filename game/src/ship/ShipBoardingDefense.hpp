// Intended function: Track breaches, security teams, sealed zones, boarding contacts, and counterattack state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::ship {
struct ShipBoardingDefenseRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ShipBoardingDefenseRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ShipBoardingDefenseNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ShipBoardingDefenseSystem {
public:
 bool submit(const ShipBoardingDefenseRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ShipBoardingDefenseRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ShipBoardingDefenseRecord> snapshot() const;
 std::vector<ShipBoardingDefenseNotice> drainNotices(); void clear();
private:
 ShipBoardingDefenseRecord* mutableFind(std::uint64_t); void notice(const ShipBoardingDefenseRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ShipBoardingDefenseRecord> records_; std::vector<ShipBoardingDefenseNotice> notices_;
};
}
