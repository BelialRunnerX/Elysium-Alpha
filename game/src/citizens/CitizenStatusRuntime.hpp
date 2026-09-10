#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::citizens {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class StackPolicy:std::uint8_t{Independent,Additive,Multiplicative,SeverityReplace,PriorityOverride,Refresh,Extend,Exclusive};enum class StatusCategory:std::uint8_t{Biological,Mind,Medical,Legal,Work,Military,Special};struct StatusDef{ContentId id{};std::uint32_t version{1};StatusCategory category{};StackPolicy stacking{};ContentId group{};std::vector<ContentId>responseAdapters;bool persistent{true};};struct ActiveStatus{StableId id{},subject{},source{};ContentId def{};std::uint64_t start{},expiry{};double severity{};std::uint32_t stacks{1};ContentId reason{};};class CitizenStatusRuntime{public:bool addDef(StatusDef,std::string&);bool apply(ActiveStatus,std::string&);void expire(std::uint64_t);std::vector<ActiveStatus>forSubject(StableId)const;private:std::unordered_map<ContentId,StatusDef>defs_;std::unordered_map<StableId,ActiveStatus>active_;};}