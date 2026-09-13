// Intended function: Project content IDs, namespaces, provenance, patches, and dependency state.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct ContentInspectorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ContentInspectorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ContentInspectorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ContentInspectorSystem {
public:
 bool submit(const ContentInspectorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ContentInspectorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ContentInspectorRecord> snapshot() const;
 std::vector<ContentInspectorNotice> drainNotices(); void clear();
private:
 ContentInspectorRecord* mutableFind(std::uint64_t); void notice(const ContentInspectorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ContentInspectorRecord> records_; std::vector<ContentInspectorNotice> notices_;
};
}
