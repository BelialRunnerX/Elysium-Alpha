// Intended function: Track secrets, discoverers, holders, exposure risk, leverage, and public revelation.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::narrative {

struct SecretSystemCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct SecretSystemState {
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

struct SecretSystemEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class SecretSystemService {
public:
    bool apply(const SecretSystemCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const SecretSystemState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<SecretSystemState> ordered() const;
    std::vector<SecretSystemEvent> drainEvents();
    void clear();

private:
    SecretSystemState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<SecretSystemState> states_;
    std::vector<SecretSystemEvent> events_;
};

} // namespace elysium::narrative
