// Intended function: Model faction sanctions, restricted commodities, enforcement strength, and economic leakage.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::diplomacy {

struct SanctionSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SanctionSystemState {
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

struct SanctionSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SanctionSystemService {
public:
    bool apply(const SanctionSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SanctionSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SanctionSystemState> ordered() const;
    std::vector<SanctionSystemEvent> drainEvents();
    void clear();

private:
    SanctionSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SanctionSystemState> states_;
    std::vector<SanctionSystemEvent> events_;
};

} // namespace elysium::diplomacy
