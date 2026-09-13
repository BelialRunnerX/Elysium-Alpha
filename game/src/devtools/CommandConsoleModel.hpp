// Intended function: Represent validated developer command requests without bypassing owner-system authority.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium::devtools {
struct CommandConsoleModelRequest { std::uint64_t id{}, owner{}, target{}, tick{}; double value{}, rate{}; std::uint32_t kind{}, flags{}; };
struct CommandConsoleModelRecord { std::uint64_t revision{}, id{}, owner{}, target{}, tick{}; double value{}, total{}, pressure{}; std::uint32_t kind{}, flags{}; bool active{false}; };
struct CommandConsoleModelNotice { std::uint64_t sequence{}, id{}, target{}, tick{}; double value{}; std::uint32_t kind{}; };
class CommandConsoleModelSystem {
public:
 bool submit(const CommandConsoleModelRequest&); bool remove(std::uint64_t); void step(std::uint64_t,double);
 [[nodiscard]] const CommandConsoleModelRecord* find(std::uint64_t) const; [[nodiscard]] std::vector<CommandConsoleModelRecord> snapshot() const;
 std::vector<CommandConsoleModelNotice> drainNotices(); void clear();
private:
 CommandConsoleModelRecord* mutableFind(std::uint64_t); void notice(const CommandConsoleModelRecord&,double,std::uint32_t);
 std::uint64_t revision_{1}, sequence_{1}; std::vector<CommandConsoleModelRecord> records_; std::vector<CommandConsoleModelNotice> notices_;
};
}
