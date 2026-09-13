#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class CaseStatus:std::uint8_t{Open,Investigating,ReadyForReview,Resolved,Dismissed};struct Evidence{StableId id{};ContentId type{};StableId source{};std::uint64_t time{};double confidence{};bool visible{true};};struct JusticeCase{StableId id{},incident{},investigator{};ContentId type{};std::vector<StableId>victims,suspects,evidence;CaseStatus status{CaseStatus::Open};};struct Verdict{StableId caseId{};bool guilty{false};ContentId action{};double confidence{};};class JusticeCaseRuntime{public:bool addCase(JusticeCase,std::string&);bool addEvidence(Evidence,std::string&);bool attach(StableId,StableId,std::string&);std::optional<Verdict>adjudicate(StableId,double threshold,ContentId action,std::string&);private:std::unordered_map<StableId,JusticeCase>cases_;std::unordered_map<StableId,Evidence>evidence_;};}