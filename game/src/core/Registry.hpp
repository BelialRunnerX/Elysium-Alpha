// Intended function: imported core implementation for Registry; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/ContentId.hpp"

#include <cstddef>
#include <cstdint>
#include <source_location>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace elysium {

// Insertion-ordered immutable-on-read registry. Reads freeze registration so a
// module cannot accidentally observe a partial catalogue and then append more
// definitions later. The vector is authoritative for order; the unordered map
// is lookup-only and is never iterated for protocol output.
template <class T>
class FrozenRegistry {
public:
    struct Entry {
        ContentId id;
        T value;
        std::string sourceFile;
        std::uint_least32_t sourceLine{};
    };

    FrozenRegistry() = default;
    FrozenRegistry(const FrozenRegistry&) = delete;
    FrozenRegistry& operator=(const FrozenRegistry&) = delete;
    FrozenRegistry(FrozenRegistry&&) = default;
    FrozenRegistry& operator=(FrozenRegistry&&) = default;

    void add(ContentId id,
             T value,
             const std::source_location where = std::source_location::current()) {
        if (frozen_) {
            std::ostringstream message;
            message << "late registry registration for '" << id.str() << "' at "
                    << where.file_name() << ':' << where.line() << " after registry freeze";
            throw std::logic_error(message.str());
        }
        const auto existing = index_.find(id.str());
        if (existing != index_.end()) {
            const auto& first = entries_[existing->second];
            std::ostringstream message;
            message << "duplicate registry id '" << id.str() << "' at "
                    << where.file_name() << ':' << where.line() << "; first registered at "
                    << first.sourceFile << ':' << first.sourceLine;
            throw std::logic_error(message.str());
        }

        const std::size_t index = entries_.size();
        entries_.push_back(Entry{std::move(id), std::move(value), where.file_name(), where.line()});
        index_.emplace(entries_.back().id.str(), index);
    }

    void freeze() const noexcept { frozen_ = true; }
    bool isFrozen() const noexcept { return frozen_; }

    std::size_t size() const noexcept {
        freeze();
        return entries_.size();
    }

    bool empty() const noexcept {
        freeze();
        return entries_.empty();
    }

    bool contains(const ContentId& id) const {
        freeze();
        return index_.contains(id.str());
    }

    const T& get(const ContentId& id) const {
        freeze();
        const auto it = index_.find(id.str());
        if (it == index_.end()) {
            throw std::out_of_range("unknown registry id '" + id.str() + "'");
        }
        return entries_[it->second].value;
    }

    std::span<const Entry> entries() const noexcept {
        freeze();
        return entries_;
    }

private:
    std::vector<Entry> entries_;
    std::unordered_map<std::string, std::size_t> index_;
    mutable bool frozen_{};
};

} // namespace elysium
