#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent deterministic automation programs as declarative rule graphs that emit intents instead of mutating simulation stores.
struct ProgrammableControllerCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct ProgrammableControllerRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class ProgrammableControllerService {
public:
    bool submit(const ProgrammableControllerCommand& command);
    const ProgrammableControllerRecord* lookup(std::uint64_t subjectId) const;
    std::vector<ProgrammableControllerRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, ProgrammableControllerRecord> records_;
};

}
