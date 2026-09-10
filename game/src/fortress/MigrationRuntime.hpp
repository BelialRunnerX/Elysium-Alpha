#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class ArrivalKind:std::uint8_t{Founder,Migrant,Refugee,Specialist,Family, Mercenary,Trader,Official,Spy,Pilgrim};enum class LegalStatus:std::uint8_t{Visitor,Resident,Citizen,Restricted,Detained,Exiled};struct Arrival{StableId id{},origin{};ArrivalKind kind{};LegalStatus legal{LegalStatus::Visitor};ContentId motive{};std::vector<ContentId>skills,possessions,knowledge;std::uint64_t intendedDuration{};};struct MigrationFactors{double reputation{},safety{},wealth{},institutions{},relations{},housing{},food{},threat{},policy{};};struct MigrationDecision{double score{};std::vector<std::pair<std::string,double>>reasons;bool eligible{false};};class MigrationRuntime{public:bool add(Arrival,std::string&);MigrationDecision evaluate(const MigrationFactors&,double threshold)const;bool promote(StableId,LegalStatus,std::string&);private:std::unordered_map<StableId,Arrival>rows_;};}