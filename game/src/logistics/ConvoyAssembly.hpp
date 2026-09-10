// Intended function: Assemble cargo and escort manifests into deterministic convoy batches.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::logistics {

struct ConvoyAssemblyCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ConvoyAssemblyState {
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

struct ConvoyAssemblyEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ConvoyAssemblyService {
public:
    bool apply(const ConvoyAssemblyCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ConvoyAssemblyState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ConvoyAssemblyState> ordered() const;
    std::vector<ConvoyAssemblyEvent> drainEvents();
    void clear();

private:
    ConvoyAssemblyState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ConvoyAssemblyState> states_;
    std::vector<ConvoyAssemblyEvent> events_;
};

} // namespace elysium::logistics
