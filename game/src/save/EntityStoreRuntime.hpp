#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::save {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class PersistenceTrait:std::uint8_t{Yes,No,Conditional,Cold,Derived};struct ComponentRecord{ContentId type{};std::uint32_t schemaVersion{1};PersistenceTrait trait{PersistenceTrait::Yes};std::vector<std::uint8_t>payload;std::vector<StableId>stableRefs;};struct EntityRecord{StableId id{};std::uint64_t revision{};std::vector<ComponentRecord>components;};class EntityStoreRuntime{public:bool put(EntityRecord,std::string&);std::optional<EntityRecord>get(StableId)const;bool resolveReference(StableId from,StableId to,std::string&)const;private:std::unordered_map<StableId,EntityRecord>rows_;};}