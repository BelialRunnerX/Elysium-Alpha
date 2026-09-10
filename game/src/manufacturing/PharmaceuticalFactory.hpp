// Intended function: Track ingredients, sterile process stages, potency, contamination, and medical output.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct PharmaceuticalFactoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct PharmaceuticalFactoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct PharmaceuticalFactoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class PharmaceuticalFactorySystem {
public:
 bool submit(const PharmaceuticalFactoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const PharmaceuticalFactoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<PharmaceuticalFactoryRecord> snapshot() const;
 std::vector<PharmaceuticalFactoryNotice> drainNotices(); void clear();
private:
 PharmaceuticalFactoryRecord* mutableFind(std::uint64_t); void notice(const PharmaceuticalFactoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<PharmaceuticalFactoryRecord> records_; std::vector<PharmaceuticalFactoryNotice> notices_;
};
}
