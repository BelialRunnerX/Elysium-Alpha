// Intended function: Track only modified chunk addresses and tombstones for delta-save publication.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct TouchedChunkIndexRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct TouchedChunkIndexRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct TouchedChunkIndexNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class TouchedChunkIndexSystem {
public:
 bool submit(const TouchedChunkIndexRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const TouchedChunkIndexRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<TouchedChunkIndexRecord> snapshot() const;
 std::vector<TouchedChunkIndexNotice> drainNotices(); void clear();
private:
 TouchedChunkIndexRecord* mutableFind(std::uint64_t); void notice(const TouchedChunkIndexRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<TouchedChunkIndexRecord> records_; std::vector<TouchedChunkIndexNotice> notices_;
};
}
