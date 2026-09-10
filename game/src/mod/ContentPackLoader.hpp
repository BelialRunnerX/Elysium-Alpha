// Intended function: Represent mod/content-pack manifests, dependencies, stable namespaces, schema requirements, and load order.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::mod {
struct ContentPackRecord {
    std::uint64_t packId{};
    std::uint64_t namespaceId{};
    std::uint64_t version{};
    std::uint64_t dependencyHash{};
    std::uint64_t schema{};
    std::uint64_t priority{};
};
class ContentPackRecordRegistry {
public:
    bool publish(ContentPackRecord record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const ContentPackRecord* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<ContentPackRecord>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const ContentPackRecord& r) noexcept;
    std::vector<ContentPackRecord> records_;
};
} // namespace elysium::mod
