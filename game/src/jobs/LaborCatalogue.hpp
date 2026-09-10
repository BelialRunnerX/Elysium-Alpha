#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::jobs {using ContentId=std::uint64_t;struct LaborDef{ContentId id{},family{},executorOwner{};std::vector<std::pair<ContentId,std::uint32_t>>requiredSkills;std::vector<ContentId>tools,ppe,stations,materials,environmentGates;bool interruptible{true};bool emergency{false};};struct WorkDetail{ContentId id{};std::unordered_set<ContentId>allowedLabor;std::int32_t priority{};};class LaborCatalogue{public:bool addLabor(LaborDef,std::string&);bool addWorkDetail(WorkDetail,std::string&);bool allowed(ContentId detail,ContentId labor)const;std::optional<LaborDef>get(ContentId)const;private:std::unordered_map<ContentId,LaborDef>labor_;std::unordered_map<ContentId,WorkDetail>details_;};}