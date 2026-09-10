// Intended function: Bind freight risk assessments to insurance coverage and claim events.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct FreightInsuranceCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct FreightInsuranceState {
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

struct FreightInsuranceEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class FreightInsuranceService {
public:
    bool apply(const FreightInsuranceCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const FreightInsuranceState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<FreightInsuranceState> ordered() const;
    std::vector<FreightInsuranceEvent> drainEvents();
    void clear();

private:
    FreightInsuranceState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<FreightInsuranceState> states_;
    std::vector<FreightInsuranceEvent> events_;
};

} // namespace elysium::logistics
