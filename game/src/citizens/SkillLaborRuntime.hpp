#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace elysium::citizens {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct SkillState{ContentId skill{};std::uint32_t rank{};double xp{};};struct EffectiveSkillContext{double focus{1},injury{1},tool{1},environment{1},mentorship{1},passive{1};};struct Eligibility{bool allowed{false};double effective{};std::string reason;};class SkillLaborRuntime{public:bool setSkills(StableId,std::vector<SkillState>);void setWorkDetail(StableId,std::unordered_set<ContentId>);Eligibility eligible(StableId,ContentId skill,ContentId jobFamily,std::uint32_t required,EffectiveSkillContext)const;bool practice(StableId,ContentId,double baseXp,double diminishing);private:std::unordered_map<StableId,std::vector<SkillState>>skills_;std::unordered_map<StableId,std::unordered_set<ContentId>>work_;};}