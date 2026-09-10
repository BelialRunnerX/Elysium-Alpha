// Intended function: Track renderer-neutral photo-mode camera, exposure, depth, time, and visibility options.
#pragma once

#include <cstdint>
#include <vector>

namespace elysium::presentation {

struct PhotoModeModelCommand {
    std::uint64_t subjectId{};
    std::uint64_t ownerId{};
    std::uint64_t targetId{};
    double amount{};
    double rate{};
    std::uint64_t tick{};
    std::uint32_t mode{};
    std::uint32_t flags{};
};

struct PhotoModeModelState {
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

struct PhotoModeModelEvent {
    std::uint64_t eventId{};
    std::uint64_t subjectId{};
    std::uint64_t targetId{};
    double magnitude{};
    std::uint64_t tick{};
    std::uint32_t kind{};
};

class PhotoModeModelService {
public:
    bool apply(const PhotoModeModelCommand& command);
    bool erase(std::uint64_t subjectId);
    void advance(std::uint64_t tick, double delta);
    [[nodiscard]] const PhotoModeModelState* find(std::uint64_t subjectId) const;
    [[nodiscard]] std::vector<PhotoModeModelState> ordered() const;
    std::vector<PhotoModeModelEvent> drainEvents();
    void clear();

private:
    PhotoModeModelState* findMutable(std::uint64_t subjectId);
    void emit(std::uint64_t subjectId, std::uint64_t targetId, double magnitude,
              std::uint64_t tick, std::uint32_t kind);

    std::uint64_t revision_{1};
    std::uint64_t nextEventId_{1};
    std::vector<PhotoModeModelState> states_;
    std::vector<PhotoModeModelEvent> events_;
};

} // namespace elysium::presentation
