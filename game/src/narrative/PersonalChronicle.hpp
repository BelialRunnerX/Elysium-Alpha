// Intended function: Build bounded character-facing histories from global Chronicle event references.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct PersonalChronicleCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PersonalChronicleState {
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

struct PersonalChronicleEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PersonalChronicleService {
public:
    bool apply(const PersonalChronicleCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PersonalChronicleState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PersonalChronicleState> ordered() const;
    std::vector<PersonalChronicleEvent> drainEvents();
    void clear();

private:
    PersonalChronicleState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PersonalChronicleState> states_;
    std::vector<PersonalChronicleEvent> events_;
};

} // namespace elysium::narrative
