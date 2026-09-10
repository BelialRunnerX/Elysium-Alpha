#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build technology graph, discovery, experiment, unlock, and research allocation projections.
struct ResearchScreenModelCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ResearchScreenModelRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ResearchScreenModelService {
public:
    bool submit(const ResearchScreenModelCommand& command);
    const ResearchScreenModelRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ResearchScreenModelRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ResearchScreenModelRecord> records_;
};

}
