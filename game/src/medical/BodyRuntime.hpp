#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::medical {using ContentId=std::uint64_t;using StableId=std::uint64_t;struct BodyPartDef{ContentId id{};ContentId parent{};std::uint32_t functionTags{};};struct Wound{StableId id{};ContentId part{},cause{};double severity{},bleeding{},pain{};bool missing{};};struct Vitals{double blood{1},oxygen{1},pain{},consciousness{1},shock{},temperature{.5},toxins{};};struct BodyState{StableId owner{};std::vector<Wound>wounds;Vitals vitals;};class BodyRuntime{public:bool registerPart(BodyPartDef);bool addBody(BodyState,std::string&);bool addWound(StableId,Wound,std::string&);double capability(StableId,std::uint32_t functionTag)const;std::optional<BodyState>get(StableId)const;private:std::unordered_map<ContentId,BodyPartDef>parts_;std::unordered_map<StableId,BodyState>bodies_;};}