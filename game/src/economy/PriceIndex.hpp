// Intended function: Maintain deterministic basket-based price indices for inflation and cost-of-living comparisons.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::economy {

struct PriceIndexCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PriceIndexState {
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

struct PriceIndexEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PriceIndexService {
public:
    bool apply(const PriceIndexCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PriceIndexState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PriceIndexState> ordered() const;
    std::vector<PriceIndexEvent> drainEvents();
    void clear();

private:
    PriceIndexState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PriceIndexState> states_;
    std::vector<PriceIndexEvent> events_;
};

} // namespace elysium::economy
