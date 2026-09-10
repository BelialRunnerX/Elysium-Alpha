#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace elysium::qa {
enum class SliceWorld:std::uint8_t{Temperate,Barren,Hazardous};
enum class SliceGate:std::uint8_t{LandMove,SeamContinuity,Mine,CraftBronze,CraftSteel,MacroBuild,MicroBuild,SealedShelter,Power,Atmosphere,Storage,RegistryBeacon,SaveReload,ShipRepair,PlanetTravel,SurvivalReadability,Combat,StandingConsequence,LodContinuity,ThreePlanetDifferentiation};
enum class GateState:std::uint8_t{NotRun,Pass,Fail,MissingOwner};
struct GateEvidence{SliceGate gate{};GateState state{GateState::NotRun};std::string ownerPackage;std::string evidenceClass;std::string reason;};
struct SliceFixture{std::uint64_t seed{};std::uint64_t temperateId{},barrenId{},hazardousId{};};
class SliceAcceptance{public:explicit SliceAcceptance(SliceFixture);bool record(GateEvidence);std::vector<GateEvidence> readiness()const;bool complete()const;std::uint64_t fingerprint()const noexcept;private:SliceFixture fixture_;std::map<SliceGate,GateEvidence> gates_;};
std::vector<SliceGate> requiredSliceGates();
}
