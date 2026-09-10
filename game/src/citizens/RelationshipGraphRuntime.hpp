#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::citizens {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct RelationshipEdge{StableId from{},to{};ContentId type{};double affinity{},trust{},loyalty{};std::uint32_t kinshipFlags{};bool known{true};std::uint64_t revision{};};struct RelationshipDelta{StableId from{},to{};double affinity{},trust{},loyalty{};ContentId provenance{};};class RelationshipGraphRuntime{public:bool upsert(RelationshipEdge,std::string&);bool apply(RelationshipDelta,std::string&);std::optional<RelationshipEdge>get(StableId,StableId)const;private:static std::uint64_t key(StableId,StableId);static double clamp(double);std::unordered_map<std::uint64_t,RelationshipEdge>edges_;};}