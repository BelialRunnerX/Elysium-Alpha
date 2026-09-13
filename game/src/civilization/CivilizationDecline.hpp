// Intended function: Aggregate demographic, military, ecological, and institutional stresses into decline risk.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::civilization {

struct CivilizationDeclineCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct CivilizationDeclineState {
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

struct CivilizationDeclineEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class CivilizationDeclineService {
public:
    bool apply(const CivilizationDeclineCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const CivilizationDeclineState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<CivilizationDeclineState> ordered() const;
    std::vector<CivilizationDeclineEvent> drainEvents();
    void clear();

private:
    CivilizationDeclineState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<CivilizationDeclineState> states_;
    std::vector<CivilizationDeclineEvent> events_;
};

} // namespace elysium::civilization
