// Intended function: Store stable localization keys, locale variants, fallbacks, formatting metadata, and mod namespace ownership.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::localization {
struct LocalizedString {
    std::uint64_t keyId{};
    std::uint64_t localeId{};
    std::uint64_t textHash{};
    std::uint64_t fallbackKey{};
    std::uint64_t namespaceId{};
    std::uint64_t flags{};
};
class LocalizedStringCollection {
public:
 bool store(LocalizedString value); bool erase(std::uint64_t id); [[nodiscard]] const LocalizedString* find(std::uint64_t id) const; [[nodiscard]] const std::vector<LocalizedString>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const LocalizedString& v) noexcept; std::vector<LocalizedString> rows_;
};
}
