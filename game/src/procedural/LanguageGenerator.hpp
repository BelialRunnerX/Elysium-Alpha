// Intended function: Generate deterministic language phonology, naming patterns, and lexical seeds per culture.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::procedural {

struct LanguageGeneratorCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct LanguageGeneratorState {
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

struct LanguageGeneratorEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class LanguageGeneratorService {
public:
    bool apply(const LanguageGeneratorCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const LanguageGeneratorState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<LanguageGeneratorState> ordered() const;
    std::vector<LanguageGeneratorEvent> drainEvents();
    void clear();

private:
    LanguageGeneratorState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<LanguageGeneratorState> states_;
    std::vector<LanguageGeneratorEvent> events_;
};

} // namespace elysium::procedural
