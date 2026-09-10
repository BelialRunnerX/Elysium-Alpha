#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule training, apprenticeships, certification, skill transfer, and research teaching across citizens.
struct EducationSystemCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct EducationSystemRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class EducationSystemService {
public:
    bool submit(const EducationSystemCommand& command);
    const EducationSystemRecord* lookup(std::uint64_t subjectId) const;
    std::vector<EducationSystemRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, EducationSystemRecord> records_;
};

}
