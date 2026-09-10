// Intended function: Track custody and access rules for strategic, hazardous, evidence, or high-value cargo.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct SecureCargoSystemRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SecureCargoSystemRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SecureCargoSystemNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SecureCargoSystemSystem {
public:
 bool submit(const SecureCargoSystemRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SecureCargoSystemRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SecureCargoSystemRecord> snapshot() const;
 std::vector<SecureCargoSystemNotice> drainNotices(); void clear();
private:
 SecureCargoSystemRecord* mutableFind(std::uint64_t); void notice(const SecureCargoSystemRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SecureCargoSystemRecord> records_; std::vector<SecureCargoSystemNotice> notices_;
};
}
