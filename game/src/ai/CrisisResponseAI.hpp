// Intended function: Choose bounded emergency responses for disasters, invasions, shortages, and epidemics.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ai {

struct CrisisResponseAICommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CrisisResponseAIState {
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

struct CrisisResponseAIEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CrisisResponseAIService {
public:
    bool apply(const CrisisResponseAICommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CrisisResponseAIState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CrisisResponseAIState> ordered() const;
    std::vector<CrisisResponseAIEvent> drainEvents();
    void clear();

private:
    CrisisResponseAIState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CrisisResponseAIState> states_;
    std::vector<CrisisResponseAIEvent> events_;
};

} // namespace elysium::ai
