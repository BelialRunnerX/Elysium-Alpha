// Intended function: Track reactor-grade components, precision, inspection, certification, and output batches.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::manufacturing {
struct ReactorComponentFactoryRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct ReactorComponentFactoryRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct ReactorComponentFactoryNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class ReactorComponentFactorySystem {
public:
 bool submit(const ReactorComponentFactoryRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const ReactorComponentFactoryRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<ReactorComponentFactoryRecord> snapshot() const;
 std::vector<ReactorComponentFactoryNotice> drainNotices(); void clear();
private:
 ReactorComponentFactoryRecord* mutableFind(std::uint64_t); void notice(const ReactorComponentFactoryRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<ReactorComponentFactoryRecord> records_; std::vector<ReactorComponentFactoryNotice> notices_;
};
}
