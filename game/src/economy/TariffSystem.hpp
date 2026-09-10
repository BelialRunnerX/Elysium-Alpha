// Intended function: Apply import/export tariffs by faction, commodity class, treaty, and strategic policy.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::economy {

struct TariffSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct TariffSystemState {
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

struct TariffSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class TariffSystemService {
public:
    bool apply(const TariffSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const TariffSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<TariffSystemState> ordered() const;
    std::vector<TariffSystemEvent> drainEvents();
    void clear();

private:
    TariffSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<TariffSystemState> states_;
    std::vector<TariffSystemEvent> events_;
};

} // namespace elysium::economy
