// Intended function: Index package metadata, dependencies, content hashes, and activation order.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::modding {

struct ModPackageIndexCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct ModPackageIndexState {
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

struct ModPackageIndexEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class ModPackageIndexService {
public:
    bool apply(const ModPackageIndexCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const ModPackageIndexState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<ModPackageIndexState> ordered() const;
    std::vector<ModPackageIndexEvent> drainEvents();
    void clear();

private:
    ModPackageIndexState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<ModPackageIndexState> states_;
    std::vector<ModPackageIndexEvent> events_;
};

} // namespace elysium::modding
