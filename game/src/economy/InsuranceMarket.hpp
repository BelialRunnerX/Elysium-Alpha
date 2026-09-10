// Intended function: Track insured assets, premiums, risk pools, claims, payouts, and solvency.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::economy {

struct InsuranceMarketCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct InsuranceMarketState {
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

struct InsuranceMarketEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class InsuranceMarketService {
public:
    bool apply(const InsuranceMarketCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const InsuranceMarketState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<InsuranceMarketState> ordered() const;
    std::vector<InsuranceMarketEvent> drainEvents();
    void clear();

private:
    InsuranceMarketState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<InsuranceMarketState> states_;
    std::vector<InsuranceMarketEvent> events_;
};

} // namespace elysium::economy
