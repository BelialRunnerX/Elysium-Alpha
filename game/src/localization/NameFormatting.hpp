// Intended function: Format generated/authored person/site/ship/artifact/faction names with titles, ranks, culture rules, and grammatical context.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::localization {
struct NameFormatState {
    std::uint64_t formatId{};
    std::uint64_t subjectId{};
    std::uint64_t cultureId{};
    std::uint64_t titleId{};
    std::uint64_t rankId{};
    std::uint64_t flags{};
};
class NameFormatStateCollection {
public:
 bool store(NameFormatState value); bool erase(std::uint64_t id); [[nodiscard]] const NameFormatState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<NameFormatState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const NameFormatState& v) noexcept; std::vector<NameFormatState> rows_;
};
}
