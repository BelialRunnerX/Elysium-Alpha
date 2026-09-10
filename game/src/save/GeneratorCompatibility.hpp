#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <vector>
namespace elysium::save {
struct GeneratorIdentity{std::uint64_t version{},fingerprint{};bool operator==(const GeneratorIdentity&)const=default;};
struct BaselineStamp{std::uint64_t universeSeed{};GeneratorIdentity generator;std::uint32_t spatialSchemaVersion{1};};
enum class CompatibilityResult:std::uint8_t{ExactMatch,MissingGenerator,FingerprintMismatch,MigrationRequired,MigrationUnavailable};
struct MigrationStep{GeneratorIdentity from,to;std::function<bool(BaselineStamp&,std::vector<std::uint8_t>&)>transform;};
class GeneratorCompatibilityRegistry{public:bool registerGenerator(GeneratorIdentity);bool registerMigration(MigrationStep);[[nodiscard]]CompatibilityResult check(const BaselineStamp&)const;CompatibilityResult migrate(BaselineStamp&,std::vector<std::uint8_t>&,GeneratorIdentity target)const;private:std::map<std::uint64_t,GeneratorIdentity>generators_;std::map<std::uint64_t,MigrationStep>migrations_;};
} // namespace elysium::save
