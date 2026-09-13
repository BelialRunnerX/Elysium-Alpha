#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct Office{StableId id{},holder{},jurisdiction{};ContentId type{},workspace{};std::vector<ContentId>responsibilities,privileges,obligations;};enum class MandateState:std::uint8_t{Active,Completed,Cancelled,Expired};struct Mandate{StableId id{},issuer{};ContentId rationale{},scope{},consequence{},legalBasis{};std::uint64_t deadline{};MandateState state{MandateState::Active};};class GovernanceRuntime{public:bool upsertOffice(Office,std::string&);bool appoint(StableId,StableId,std::string&);bool addMandate(Mandate,std::string&);bool resolveMandate(StableId,MandateState,std::string&);std::optional<Office>office(StableId)const;private:std::unordered_map<StableId,Office>offices_;std::unordered_map<StableId,Mandate>mandates_;};}