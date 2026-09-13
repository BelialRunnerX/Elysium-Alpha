#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::faction {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct FileFact{StableId id{};ContentId type{};StableId subject{},object{},source{};double confidence{};std::uint64_t time{};std::vector<StableId>contradicts,supersedes;std::uint32_t legalTags{};};class ImperialFileRuntime{public:bool add(FileFact,bool legitimateSource,std::string&);std::vector<FileFact>factsFor(StableId)const;std::optional<FileFact>bestFact(StableId,ContentId)const;private:std::unordered_map<StableId,FileFact>facts_;};}