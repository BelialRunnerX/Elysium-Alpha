#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent graphics, audio, controls, accessibility, gameplay, and UI settings independently of platform storage APIs.
struct SettingsStoreCommand {
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
};

struct SettingsStoreRecord {
    std::uint64_t revision{0};
    std::uint64_t sourceId{0};
    std::uint64_t subjectId{0};
    std::uint64_t auxId{0};
    double scalar{0.0};
    std::uint32_t mode{0};
    bool enabled{false};
};

class SettingsStoreService {
public:
    bool submit(const SettingsStoreCommand& command);
    const SettingsStoreRecord* lookup(std::uint64_t subjectId) const;
    std::vector<SettingsStoreRecord> snapshot() const;
    bool remove(std::uint64_t subjectId);
    void reset();
private:
    std::uint64_t revision_{1};
    std::unordered_map<std::uint64_t, SettingsStoreRecord> records_;
};

}
