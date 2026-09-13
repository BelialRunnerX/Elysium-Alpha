// Intended function: Model administrative capacity, service reach, corruption load, and policy execution limits.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct InstitutionalCapacityCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct InstitutionalCapacityState {
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

struct InstitutionalCapacityEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class InstitutionalCapacityService {
public:
    bool apply(const InstitutionalCapacityCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const InstitutionalCapacityState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<InstitutionalCapacityState> ordered() const;
    std::vector<InstitutionalCapacityEvent> drainEvents();
    void clear();

private:
    InstitutionalCapacityState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<InstitutionalCapacityState> states_;
    std::vector<InstitutionalCapacityEvent> events_;
};

} // namespace elysium::civilization
