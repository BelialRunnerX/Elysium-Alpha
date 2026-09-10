// Intended function: Generate clothing and equipment aesthetic motifs from culture, climate, status, and materials.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct FashionGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct FashionGeneratorState {
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

struct FashionGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class FashionGeneratorService {
public:
    bool apply(const FashionGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const FashionGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<FashionGeneratorState> ordered() const;
    std::vector<FashionGeneratorEvent> drainEvents();
    void clear();

private:
    FashionGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<FashionGeneratorState> states_;
    std::vector<FashionGeneratorEvent> events_;
};

} // namespace elysium::procedural
