// Intended function: Track persistent stable-ID object locations, shards, tombstones, and version metadata.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct StableObjectIndexRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct StableObjectIndexRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct StableObjectIndexNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class StableObjectIndexSystem {
public:
 bool submit(const StableObjectIndexRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const StableObjectIndexRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<StableObjectIndexRecord> snapshot() const;
 std::vector<StableObjectIndexNotice> drainNotices(); void clear();
private:
 StableObjectIndexRecord* mutableFind(std::uint64_t); void notice(const StableObjectIndexRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<StableObjectIndexRecord> records_; std::vector<StableObjectIndexNotice> notices_;
};
}
