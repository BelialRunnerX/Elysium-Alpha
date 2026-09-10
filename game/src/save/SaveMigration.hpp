// Intended function: ordered save-schema migration registry with explicit version steps so old world history is transformed intentionally rather than silently discarded.
#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
namespace elysium{
struct SaveMigrationContext{std::uint64_t worldId{};std::uint32_t fromVersion{},toVersion{};std::string payload;std::vector<std::string>warnings;};
using SaveMigrationFn=std::function<bool(SaveMigrationContext&)>;
struct SaveMigrationStep{std::uint32_t fromVersion{},toVersion{};std::string name;SaveMigrationFn migrate;};
class SaveMigrationRegistry{public:bool add(SaveMigrationStep step);std::optional<SaveMigrationContext>migrate(std::uint64_t worldId,std::uint32_t from,std::uint32_t to,std::string payload)const;private:std::vector<SaveMigrationStep>steps_;};
}
