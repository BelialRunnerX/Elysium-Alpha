#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::security {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct WitnessStatement{StableId id{},witness{},subject{};std::uint64_t time{},location{};ContentId claim{};double confidence{},perception{},memory{},bias{},loyalty{},relationship{};};struct SensorEvidence{StableId id{},sensor{},subject{};ContentId claim{};std::uint64_t time{},location{};double confidence{};};struct InfiltrationIdentity{StableId actor{};ContentId method{},cover{};double compromise{};std::vector<ContentId>observableBehaviors,countermeasures;};class SecurityEvidenceRuntime{public:bool addWitness(WitnessStatement,std::string&);bool addSensor(SensorEvidence,bool powered,bool covered,std::string&);bool upsertInfiltration(InfiltrationIdentity,std::string&);std::vector<StableId>evidenceFor(StableId subject)const;private:std::unordered_map<StableId,WitnessStatement>witness_;std::unordered_map<StableId,SensorEvidence>sensors_;std::unordered_map<StableId,InfiltrationIdentity>infiltration_;};}