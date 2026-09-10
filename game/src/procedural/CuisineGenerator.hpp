// Intended function: Generate cuisines from local ingredients, culture, preservation technology, and trade access.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct CuisineGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CuisineGeneratorState {
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

struct CuisineGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CuisineGeneratorService {
public:
    bool apply(const CuisineGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CuisineGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CuisineGeneratorState> ordered() const;
    std::vector<CuisineGeneratorEvent> drainEvents();
    void clear();

private:
    CuisineGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CuisineGeneratorState> states_;
    std::vector<CuisineGeneratorEvent> events_;
};

} // namespace elysium::procedural
