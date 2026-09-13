// Intended function: Track detainee location, status, security needs, medical needs, hearings, and transfers.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct PrisonerCustodyRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PrisonerCustodyRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PrisonerCustodyNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PrisonerCustodySystem {
public:
 bool submit(const PrisonerCustodyRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PrisonerCustodyRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PrisonerCustodyRecord> snapshot() const;
 std::vector<PrisonerCustodyNotice> drainNotices(); void clear();
private:
 PrisonerCustodyRecord* mutableFind(std::uint64_t); void notice(const PrisonerCustodyRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PrisonerCustodyRecord> records_; std::vector<PrisonerCustodyNotice> notices_;
};
}
