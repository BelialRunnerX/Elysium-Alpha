// Intended function: Record durable security incidents and cross-links for justice/history consumers.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct SecurityIncidentLogRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SecurityIncidentLogRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SecurityIncidentLogNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SecurityIncidentLogSystem {
public:
 bool submit(const SecurityIncidentLogRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SecurityIncidentLogRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SecurityIncidentLogRecord> snapshot() const;
 std::vector<SecurityIncidentLogNotice> drainNotices(); void clear();
private:
 SecurityIncidentLogRecord* mutableFind(std::uint64_t); void notice(const SecurityIncidentLogRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SecurityIncidentLogRecord> records_; std::vector<SecurityIncidentLogNotice> notices_;
};
}
