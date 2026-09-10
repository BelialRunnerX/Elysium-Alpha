// Intended function: Track required generator/content/mod versions needed to load durable state safely.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct ContentCompatibilityRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ContentCompatibilityRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ContentCompatibilityNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ContentCompatibilitySystem {
public:
 bool submit(const ContentCompatibilityRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ContentCompatibilityRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ContentCompatibilityRecord> snapshot() const;
 std::vector<ContentCompatibilityNotice> drainNotices(); void clear();
private:
 ContentCompatibilityRecord* mutableFind(std::uint64_t); void notice(const ContentCompatibilityRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ContentCompatibilityRecord> records_; std::vector<ContentCompatibilityNotice> notices_;
};
}
