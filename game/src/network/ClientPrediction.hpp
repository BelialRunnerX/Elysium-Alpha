// Intended function: Track prediction inputs, sequence numbers, reconciliation state, and correction magnitude.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::network {

struct ClientPredictionCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ClientPredictionState {
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

struct ClientPredictionEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ClientPredictionService {
public:
    bool apply(const ClientPredictionCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ClientPredictionState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ClientPredictionState> ordered() const;
    std::vector<ClientPredictionEvent> drainEvents();
    void clear();

private:
    ClientPredictionState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ClientPredictionState> states_;
    std::vector<ClientPredictionEvent> events_;
};

} // namespace elysium::network
