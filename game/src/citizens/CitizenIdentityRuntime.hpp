#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::citizens {using StableId=std::uint64_t;enum class LifeStage:std::uint8_t{Child,Adolescent,Adult,Elder};enum class LifeState:std::uint8_t{Active,Incapacitated,Retired,Departed,Dead};enum class PersistencePolicy:std::uint8_t{CohortCompactable,NamedPersistent,HistoryPersistent};struct CitizenIdentity{StableId id{},nameId{},speciesId{},household{};LifeStage stage{LifeStage::Adult};LifeState state{LifeState::Active};PersistencePolicy persistence{PersistencePolicy::CohortCompactable};std::uint64_t shard{},lastStrategicUpdate{};bool historySignificant{false};std::vector<StableId>relationships;};struct LifeTransition{StableId citizen{};LifeState from{},to{};std::uint64_t tick{};std::vector<std::string>consequences;};class CitizenIdentityRuntime{public:bool add(CitizenIdentity,std::string&);std::optional<LifeTransition>transition(StableId,LifeState,std::uint64_t,std::string&);bool canCompact(StableId)const;private:std::unordered_map<StableId,CitizenIdentity>rows_;};}