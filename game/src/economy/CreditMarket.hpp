// Intended function: Track loans, principal, interest, collateral pressure, defaults, and lending capacity.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::economy {

struct CreditMarketCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CreditMarketState {
    std::uint64_t revision{};
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double accumulated{};
    double pressure{};
    std::uint64_t updatedTick{};
    std::uint32_t mode{};
    std::uint32_t status{};
    bool active{false};
};

struct CreditMarketEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CreditMarketService {
public:
    bool apply(const CreditMarketCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CreditMarketState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CreditMarketState> ordered() const;
    std::vector<CreditMarketEvent> drainEvents();
    void clear();

private:
    CreditMarketState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CreditMarketState> states_;
    std::vector<CreditMarketEvent> events_;
};

} // namespace elysium::economy
