#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Resolve stable text keys, locale fallbacks, plural forms, substitutions, and content-pack namespaces.
struct LocalizationDatabaseCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct LocalizationDatabaseRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class LocalizationDatabaseService {
public:
    bool submit(const LocalizationDatabaseCommand& command);
    const LocalizationDatabaseRecord* lookup(std::uint64_t subjectId) const;
    std::vector<LocalizationDatabaseRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, LocalizationDatabaseRecord> records_;
};

}
