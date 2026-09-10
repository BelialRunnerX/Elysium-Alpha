// Intended function: Track planetary radiation belt intensity, shielding demand, and route exposure.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::stellar {

struct RadiationBeltsCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct RadiationBeltsState {
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

struct RadiationBeltsEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class RadiationBeltsService {
public:
    bool apply(const RadiationBeltsCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const RadiationBeltsState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<RadiationBeltsState> ordered() const;
    std::vector<RadiationBeltsEvent> drainEvents();
    void clear();

private:
    RadiationBeltsState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<RadiationBeltsState> states_;
    std::vector<RadiationBeltsEvent> events_;
};

} // namespace elysium::stellar
