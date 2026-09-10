// Intended function: Record save/migration/recovery operations for diagnostics without exposing secret data.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::saveinfra {
struct SaveAuditTrailRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct SaveAuditTrailRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct SaveAuditTrailNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class SaveAuditTrailSystem {
public:
 bool submit(const SaveAuditTrailRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const SaveAuditTrailRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<SaveAuditTrailRecord> snapshot() const;
 std::vector<SaveAuditTrailNotice> drainNotices(); void clear();
private:
 SaveAuditTrailRecord* mutableFind(std::uint64_t); void notice(const SaveAuditTrailRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<SaveAuditTrailRecord> records_; std::vector<SaveAuditTrailNotice> notices_;
};
}
