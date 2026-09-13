// Intended function: Track gas extraction/injection, composition targets, energy demand, and regional progress.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::planetary {
struct AtmosphereProcessorRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct AtmosphereProcessorRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct AtmosphereProcessorNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class AtmosphereProcessorSystem {
public:
 bool submit(const AtmosphereProcessorRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const AtmosphereProcessorRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<AtmosphereProcessorRecord> snapshot() const;
 std::vector<AtmosphereProcessorNotice> drainNotices(); void clear();
private:
 AtmosphereProcessorRecord* mutableFind(std::uint64_t); void notice(const AtmosphereProcessorRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<AtmosphereProcessorRecord> records_; std::vector<AtmosphereProcessorNotice> notices_;
};
}
