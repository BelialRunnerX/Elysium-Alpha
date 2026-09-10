#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::ecs {using ContentId=std::uint64_t;enum class PersistenceClass:std::uint8_t{Yes,No,Conditional,Cold,Derived};struct ComponentMeta{ContentId type{};std::uint32_t schemaVersion{1};ContentId owner{};PersistenceClass persistence{};bool containsRawEntity{},rawPointer{},jobHandle{},gpuHandle{},iterationIndex{};std::vector<ContentId>rebuildInputs;std::vector<std::string>shards;std::string whyExposure;};class ComponentMetadataCatalogue{public:bool add(ComponentMeta,std::string&);bool lint(std::vector<std::string>&)const;std::optional<ComponentMeta>get(ContentId)const;private:std::unordered_map<ContentId,ComponentMeta>rows_;};}