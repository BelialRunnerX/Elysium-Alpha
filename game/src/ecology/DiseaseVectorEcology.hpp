// Intended function: Track vector populations, habitat suitability, disease pressure, and control measures.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::ecology {

struct DiseaseVectorEcologyCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct DiseaseVectorEcologyState {
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

struct DiseaseVectorEcologyEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class DiseaseVectorEcologyService {
public:
    bool apply(const DiseaseVectorEcologyCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const DiseaseVectorEcologyState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<DiseaseVectorEcologyState> ordered() const;
    std::vector<DiseaseVectorEcologyEvent> drainEvents();
    void clear();

private:
    DiseaseVectorEcologyState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<DiseaseVectorEcologyState> states_;
    std::vector<DiseaseVectorEcologyEvent> events_;
};

} // namespace elysium::ecology
