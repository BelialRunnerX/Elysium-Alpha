#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::contracts { using ContentId=std::uint64_t;using StableId=std::uint64_t; enum class ContractFamily:std::uint8_t{Survey,Procurement,Construction,Recovery,Escort,Defense,Diplomatic,Research}; struct ObjectiveSpec{ContentId type{};std::uint32_t minCount{1},maxCount{1};}; struct ContractTemplate{ContentId id{};std::uint32_t version{1};ContractFamily family{};std::vector<ObjectiveSpec> objectives;std::vector<ContentId> complicationPool;ContentId rewardTable{};std::uint32_t maxComplications{0};}; struct ContractInstance{StableId id{};ContentId templateId{};std::uint64_t seed{};std::vector<ObjectiveSpec> objectives;std::vector<ContentId> complications;bool settled{false};}; class ContractTemplateRuntime{public:bool add(ContractTemplate,std::string&);std::optional<ContractInstance> compile(ContentId,std::uint64_t seed,std::uint64_t issuer,std::string&)const;private:std::unordered_map<ContentId,ContractTemplate> defs_;}; }