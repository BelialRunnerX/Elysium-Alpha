#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct CultureProfile{ContentId id{};std::vector<std::pair<ContentId,double>>values,materialPreferences,cuisinePreferences,artPreferences;double empireAttitude{},unswornAttitude{},riftAttitude{};};struct Institution{StableId id{};ContentId type{},culture{};std::vector<StableId>rooms,staff,members;std::vector<ContentId>supplies,activities,petitions;double quality{},prestige{};std::uint64_t scheduleMask{};};class InstitutionRuntime{public:bool addCulture(CultureProfile,std::string&);bool upsert(Institution,std::string&);bool qualified(StableId)const;std::optional<Institution>get(StableId)const;private:std::unordered_map<ContentId,CultureProfile>cultures_;std::unordered_map<StableId,Institution>inst_;};}