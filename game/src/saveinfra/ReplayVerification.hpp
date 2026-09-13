// Intended function: Track deterministic replay hashes, divergence locations, and subsystem summaries.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct ReplayVerificationRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ReplayVerificationRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ReplayVerificationNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ReplayVerificationSystem {
public:
 bool submit(const ReplayVerificationRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ReplayVerificationRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ReplayVerificationRecord> snapshot() const;
 std::vector<ReplayVerificationNotice> drainNotices(); void clear();
private:
 ReplayVerificationRecord* mutableFind(std::uint64_t); void notice(const ReplayVerificationRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ReplayVerificationRecord> records_; std::vector<ReplayVerificationNotice> notices_;
};
}
