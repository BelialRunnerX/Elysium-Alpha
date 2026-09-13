// Intended function: imported core implementation for ContentId; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

namespace elysium {

// Namespaced content identity used across saves/builds. This is intentionally
// not an enum ordinal or registry insertion index. The syntax is deliberately
// conservative so IDs have one canonical textual representation.
class ContentId {
public:
    explicit ContentId(std::string_view value) : value_(value) {
        validate(value_);
    }

    const std::string& str() const noexcept { return value_; }
    std::string_view nameSpace() const noexcept {
        return std::string_view(value_).substr(0, value_.find(':'));
    }
    std::string_view path() const noexcept {
        const auto split = value_.find(':');
        return std::string_view(value_).substr(split + 1);
    }

    bool operator==(const ContentId&) const = default;
    bool operator<(const ContentId& other) const noexcept { return value_ < other.value_; }

    static bool isValid(std::string_view value) noexcept {
        const auto split = value.find(':');
        if (split == std::string_view::npos || split == 0 || split + 1 >= value.size()) return false;
        if (value.find(':', split + 1) != std::string_view::npos) return false;

        auto namespaceChar = [](char c) {
            return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
        };
        auto pathChar = [&](char c) { return namespaceChar(c) || c == '/'; };

        for (std::size_t i = 0; i < split; ++i) if (!namespaceChar(value[i])) return false;
        for (std::size_t i = split + 1; i < value.size(); ++i) if (!pathChar(value[i])) return false;

        const auto path = value.substr(split + 1);
        if (path.front() == '/' || path.back() == '/' || path.find("//") != std::string_view::npos) return false;
        return true;
    }

private:
    std::string value_;

    static void validate(std::string_view value) {
        if (!isValid(value)) {
            throw std::invalid_argument("invalid namespaced ContentId '" + std::string(value) + "'");
        }
    }
};

} // namespace elysium
