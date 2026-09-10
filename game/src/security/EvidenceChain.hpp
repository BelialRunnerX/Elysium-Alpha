// Intended function: Track evidence provenance, custody, integrity, related cases, and admissibility.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::security {
struct EvidenceChainRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct EvidenceChainRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct EvidenceChainNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class EvidenceChainSystem {
public:
 bool submit(const EvidenceChainRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const EvidenceChainRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<EvidenceChainRecord> snapshot() const;
 std::vector<EvidenceChainNotice> drainNotices(); void clear();
private:
 EvidenceChainRecord* mutableFind(std::uint64_t); void notice(const EvidenceChainRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<EvidenceChainRecord> records_; std::vector<EvidenceChainNotice> notices_;
};
}
