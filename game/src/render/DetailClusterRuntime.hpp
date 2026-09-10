#pragma once
#include <cstdint>
#include <optional>
#include <unordered_set>
#include <vector>
namespace elysium::detail {
using StableId=std::uint64_t; using ContentId=std::uint64_t;
struct ClusterSeed { StableId worldId{}; std::uint64_t spatialKey{}; ContentId content{}; std::uint64_t universeSeed{}; };
struct Instance { StableId stableId{}; float importance{}; float maxDistance{}; std::uint32_t triangleCost{}; };
struct Budget { std::uint32_t maxInstances{4096}; std::uint64_t maxTriangles{200000}; double density{1.0}; };
enum class Interaction : std::uint8_t { Burn,Dig,Destroy,Harvest,BuildOver };
enum class DeltaKind : std::uint8_t { Tombstone,Harvested,Scorched,Replaced };
struct PromotionCommand { StableId detailId{}; StableId worldId{}; std::uint64_t spatialKey{}; DeltaKind kind{DeltaKind::Tombstone}; std::uint64_t transactionId{}; };
std::vector<Instance> generateCluster(ClusterSeed, std::uint32_t nominalCount);
std::vector<Instance> thinCluster(const std::vector<Instance>&, Budget);
class PromotionLedger { public: std::optional<PromotionCommand> promote(ClusterSeed, StableId detailId, Interaction, std::uint64_t transactionId); bool promoted(StableId id) const; private: std::unordered_set<StableId> ids_; std::unordered_set<std::uint64_t> transactions_; };
}
