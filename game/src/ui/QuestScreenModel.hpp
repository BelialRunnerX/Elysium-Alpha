#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build quest graph, objectives, rewards, failures, history, and tracked-marker projections.
struct QuestScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct QuestScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class QuestScreenModelService {
public:
    bool submit(const QuestScreenModelCommand& command);
    const QuestScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<QuestScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, QuestScreenModelRecord> records_;
};

}
