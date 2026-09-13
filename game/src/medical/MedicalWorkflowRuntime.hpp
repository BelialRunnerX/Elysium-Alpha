#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::medical {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class TreatmentStage:std::uint8_t{Rescue,Stabilize,Decontaminate,Surgery,FractureCare,Dress,Medication,Monitor,Rehab,ReturnToWork,Complete};struct MedicalCase{StableId id{},patient{};TreatmentStage stage{TreatmentStage::Rescue};double threat{},capabilityRisk{},contamination{};bool transportable{true};std::vector<StableId>reservedResources;};class MedicalWorkflowRuntime{public:bool add(MedicalCase,std::string&);std::vector<StableId>triageOrder()const;bool advance(StableId,TreatmentStage,const std::vector<StableId>&required,std::string&);private:std::unordered_map<StableId,MedicalCase>cases_;std::unordered_map<StableId,StableId>reservations_;};}