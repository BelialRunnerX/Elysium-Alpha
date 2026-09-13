// Intended function: imported core implementation for GeneratorContract; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "core/Determinism.hpp"

#include <cstdint>
#include <source_location>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace elysium {

// Save-compatibility identity for a procedural baseline. Version and
// fingerprint are intentionally separate: a human-readable version selects a
// reader/generator implementation, while the fingerprint detects accidental
// drift under that version.
struct GeneratorContract {
    std::uint32_t version{};
    std::uint64_t fingerprint{};

    constexpr bool operator==(const GeneratorContract&) const = default;
    constexpr bool valid() const noexcept { return version != 0 && fingerprint != 0; }
};

struct GeneratorRegressionBaseline {
    GeneratorContract contract{};
    std::uint64_t outputDigest{};
};

// A fixed regression baseline only constrains an unchanged declared contract.
// Once version/fingerprint intentionally advances, the caller supplies a new
// baseline while retaining old generator support according to save policy.
constexpr bool generatorRegressionAccepts(GeneratorContract current,
                                          std::uint64_t currentDigest,
                                          GeneratorRegressionBaseline baseline) noexcept {
    return current != baseline.contract || currentDigest == baseline.outputDigest;
}

inline void requireValidGeneratorContract(GeneratorContract contract, const char* diagnosticName) {
    if (!contract.valid()) {
        throw std::invalid_argument(std::string("invalid generator contract for ") + diagnosticName);
    }
}

struct GeneratorLabelRecord {
    std::string name;
    SeedLabel seedLabel{};
    // Generator version in which the current interpretation of this label was
    // introduced. Changing a label's meaning therefore requires consciously
    // naming a generator version rather than silently reusing an old stream.
    std::uint32_t meaningVersion{};
};

// Small protocol ledger for a generator's labeled random streams. It rejects
// name/token reuse and rejects a semantic label revision that claims a version
// newer than the generator contract. A fixed output corpus remains the final
// guard against changing an old label's algorithm while forgetting to update
// its declared version/fingerprint.
class GeneratorLabelLedger {
public:
    explicit GeneratorLabelLedger(GeneratorContract contract) : contract_(contract) {
        requireValidGeneratorContract(contract_, "generator label ledger");
    }

    void add(std::string name,
             SeedLabel seedLabel,
             std::uint32_t meaningVersion,
             const std::source_location where = std::source_location::current()) {
        if (frozen_) throw std::logic_error("late generator-label registration after ledger freeze");
        if (name.empty()) throw std::invalid_argument("generator label name may not be empty");
        if (seedLabel.token == 0) throw std::invalid_argument("generator label token may not be zero");
        if (meaningVersion == 0 || meaningVersion > contract_.version) {
            std::ostringstream message;
            message << "generator label '" << name << "' meaning version " << meaningVersion
                    << " is incompatible with generator version " << contract_.version
                    << " at " << where.file_name() << ':' << where.line();
            throw std::logic_error(message.str());
        }
        if (names_.contains(name)) {
            throw std::logic_error("duplicate generator label name '" + name + "'");
        }
        if (tokens_.contains(seedLabel.token)) {
            throw std::logic_error("generator label token reused by '" + name + "'");
        }
        names_.emplace(name, records_.size());
        tokens_.insert(seedLabel.token);
        records_.push_back({std::move(name), seedLabel, meaningVersion});
    }

    const std::vector<GeneratorLabelRecord>& records() const noexcept {
        frozen_ = true;
        return records_;
    }

    std::uint64_t protocolDigest() const noexcept {
        frozen_ = true;
        std::uint64_t digest = 0xCBF29CE484222325ULL;
        for (const auto& record : records_) {
            digest = stableHashCombine(digest, stableStringHash(record.name));
            digest = stableHashCombine(digest, record.seedLabel.token);
            digest = stableHashCombine(digest, record.meaningVersion);
        }
        return digest;
    }

    bool isFrozen() const noexcept { return frozen_; }
    GeneratorContract contract() const noexcept { return contract_; }

private:
    GeneratorContract contract_{};
    std::vector<GeneratorLabelRecord> records_;
    std::unordered_map<std::string, std::size_t> names_;
    std::unordered_set<std::uint64_t> tokens_;
    mutable bool frozen_{};
};

} // namespace elysium
