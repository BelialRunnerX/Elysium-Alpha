#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::spatial {enum class ServiceKind:std::uint8_t{RoomGraph,SupportGraph,NavField,FluidVolume,ThermalField,ContaminationField,VisibilityField};struct Region{std::int32_t x0{},y0{},z0{},x1{},y1{},z1{};};struct Snapshot{ServiceKind kind{};std::uint64_t revision{};Region region{};};struct DirtyRequest{ServiceKind kind{};Region region{};std::uint64_t causeRevision{};};class SpatialServiceRegistry{public:void declareDependency(ServiceKind from,ServiceKind to);void invalidate(DirtyRequest);std::vector<DirtyRequest>drainDirty();bool publish(Snapshot,std::string&);std::optional<Snapshot>snapshot(ServiceKind)const;private:std::unordered_map<ServiceKind,std::vector<ServiceKind>>deps_;std::vector<DirtyRequest>dirty_;std::unordered_map<ServiceKind,Snapshot>snapshots_;};}