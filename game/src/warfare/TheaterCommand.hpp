// Intended function: Coordinate multiple fronts under a theater-level objective and bounded strategic budget.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::warfare {

struct TheaterCommandCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct TheaterCommandState {
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

struct TheaterCommandEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class TheaterCommandService {
public:
    bool apply(const TheaterCommandCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const TheaterCommandState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<TheaterCommandState> ordered() const;
    std::vector<TheaterCommandEvent> drainEvents();
    void clear();

private:
    TheaterCommandState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<TheaterCommandState> states_;
    std::vector<TheaterCommandEvent> events_;
};

} // namespace elysium::warfare
