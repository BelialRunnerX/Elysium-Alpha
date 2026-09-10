// Intended function: Store gameplay, accessibility, camera, UI, audio, graphics, simulation-budget, and debug settings as versioned data.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::game {
struct SettingsRecord {
    std::uint64_t settingId{};
    std::uint64_t category{};
    double value{};
    double minValue{};
    double maxValue{};
    std::uint64_t flags{};
};
class SettingsRecordStore {
public:
 bool put(SettingsRecord v); bool erase(std::uint64_t id);
 [[nodiscard]] const SettingsRecord* get(std::uint64_t id) const;
 [[nodiscard]] const std::vector<SettingsRecord>& all() const noexcept { return values_; }
private:
 static std::uint64_t idOf(const SettingsRecord& v) noexcept; std::vector<SettingsRecord> values_;
};
} // namespace elysium::game
