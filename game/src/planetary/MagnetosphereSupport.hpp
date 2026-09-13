// Intended function: Track orbital/ground field generators, coverage, power, radiation shielding, and failures.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct MagnetosphereSupportRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct MagnetosphereSupportRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct MagnetosphereSupportNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class MagnetosphereSupportSystem {
public:
 bool submit(const MagnetosphereSupportRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const MagnetosphereSupportRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<MagnetosphereSupportRecord> snapshot() const;
 std::vector<MagnetosphereSupportNotice> drainNotices(); void clear();
private:
 MagnetosphereSupportRecord* mutableFind(std::uint64_t); void notice(const MagnetosphereSupportRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<MagnetosphereSupportRecord> records_; std::vector<MagnetosphereSupportNotice> notices_;
};
}
