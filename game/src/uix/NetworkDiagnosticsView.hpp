// Intended function: Project future session, latency, baselines, prediction, replication, and desync summaries.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::uix {
struct NetworkDiagnosticsViewRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct NetworkDiagnosticsViewRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct NetworkDiagnosticsViewNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class NetworkDiagnosticsViewSystem {
public:
 bool submit(const NetworkDiagnosticsViewRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const NetworkDiagnosticsViewRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<NetworkDiagnosticsViewRecord> snapshot() const;
 std::vector<NetworkDiagnosticsViewNotice> drainNotices(); void clear();
private:
 NetworkDiagnosticsViewRecord* mutableFind(std::uint64_t); void notice(const NetworkDiagnosticsViewRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<NetworkDiagnosticsViewRecord> records_; std::vector<NetworkDiagnosticsViewNotice> notices_;
};
}
