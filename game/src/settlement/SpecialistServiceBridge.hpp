#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::settlement {
enum class SpecialistRole:std::uint8_t{Repair,TradeAccess,CropTending,ResearchAnalysis};
enum class SpecialistFailure:std::uint8_t{None,Invalid,DuplicateAssignment,RoleCapacity,MissingInput,NoLocalNetwork,NoPower,NoRoomAccess};
struct SpecialistActor{std::uint64_t stableId{},homeSettlementId{},assignmentStableId{},revision{1};SpecialistRole role{};std::uint64_t relationshipRef{},serviceState{};bool sleeping{};};
struct SpecialistLocalContext{std::uint64_t settlementStableId{},localNetworkId{},roomStableId{},inputCapabilityMask{};bool powered{},roomAccess{};};
struct SpecialistServiceRequest{std::uint64_t stableId{},specialistStableId{},requiredInputMask{};std::uint64_t targetStableId{};};
struct SpecialistServiceResult{bool accepted{};SpecialistFailure failure{};std::uint64_t specialistStableId{},targetStableId{};SpecialistRole role{};};
struct FutureJobAdapter{std::uint64_t workerStableId{},assignmentStableId{};std::uint64_t capabilityMask{};};
class SpecialistServiceBridge{public:bool add(SpecialistActor);SpecialistFailure assign(std::uint64_t specialistId,std::uint64_t assignmentId,std::uint64_t rolePopulation,std::uint64_t roleCapacity);SpecialistServiceResult request(const SpecialistServiceRequest&,const SpecialistLocalContext&)const;[[nodiscard]]FutureJobAdapter jobAdapter(std::uint64_t)const;[[nodiscard]]const SpecialistActor*find(std::uint64_t)const;private:std::map<std::uint64_t,SpecialistActor>actors_;};
} // namespace elysium::settlement
