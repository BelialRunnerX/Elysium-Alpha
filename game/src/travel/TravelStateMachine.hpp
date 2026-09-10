#pragma once
#include <cstdint>
#include <optional>
#include <string>
namespace elysium::travel {
using StableId=std::uint64_t; enum class TravelState:std::uint8_t{SurfaceLanded,AtmosphericAscent,NearOrbit,PulseTransit,SystemFreeFlight,WarpCharge,WarpTransition,Interception,Docked};
struct TravelContext { bool flightAllowed{true}; bool dockingAvailable{false}; bool hasDockingCollar{false}; bool routeKnown{false}; bool interdicted{false}; double fuel{0}; double warpCost{0}; };
struct TravelRecord { StableId ship{}; TravelState state{TravelState::SurfaceLanded}; StableId sourceSystem{},destinationSystem{},body{},port{}; std::uint64_t transitionVersion{}; bool costSettled{false}; };
struct TransitionResult { bool accepted{false}; std::string reason; double fuelConsumed{0}; TravelState state{}; };
class TravelStateMachine { public: explicit TravelStateMachine(TravelRecord r):record_(r){} TransitionResult transition(TravelState to,TravelContext& ctx); const TravelRecord& record()const{return record_;} void recoverFromTornState(); private: bool legal(TravelState from,TravelState to)const; TravelRecord record_; };
}