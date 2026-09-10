#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;
struct BiomeProductionSheet {
  ContentId biome{}; std::uint32_t schemaVersion{1}; ContentId planetClass{};
  std::vector<float> climateTarget; std::vector<ContentId> surfaceStack;
  ContentId terrainModifier{}, floraSet{}, faunaModifier{}, weatherTable{}, resourceModifier{}, poiModifier{}, detailProfile{}, constructionPressure{};
  bool anomalousNonClaimable{false}; std::vector<std::string> operationalTags;
};
struct BiomeProjection { ContentId biome{}; std::uint32_t schemaVersion{}; bool known{false}; bool anomalousNonClaimable{false}; std::vector<std::string> operationalTags; };
class BiomeProductionRegistry {
 public:
  bool add(BiomeProductionSheet sheet, std::string& reason);
  bool validateReferences(const std::unordered_set<ContentId>& known, std::string& reason) const;
  bool freeze(std::string& reason); bool frozen() const noexcept { return frozen_; }
  std::optional<BiomeProjection> project(ContentId biome, bool discovered) const;
  std::vector<ContentId> orderedIds() const;
 private: bool frozen_{false}; std::unordered_map<ContentId,BiomeProductionSheet> rows_;
};
}