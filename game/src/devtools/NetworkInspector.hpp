// Intended function: Project future replication baselines, interest sets, prediction, and desync summaries.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct NetworkInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct NetworkInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct NetworkInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class NetworkInspectorSystem {
public:
 bool submit(const NetworkInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const NetworkInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<NetworkInspectorRecord> snapshot() const;
 std::vector<NetworkInspectorNotice> drainNotices(); void clear();
private:
 NetworkInspectorRecord* mutableFind(std::uint64_t); void notice(const NetworkInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<NetworkInspectorRecord> records_; std::vector<NetworkInspectorNotice> notices_;
};
}
