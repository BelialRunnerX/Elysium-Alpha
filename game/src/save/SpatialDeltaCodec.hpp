#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::save {
enum class SpatialDomain:std::uint8_t{PlanetChunk,OrbitalPartition,DerelictInterior,RiftCell};
enum class SpatialDeltaKind:std::uint8_t{MacrovoxelOverride,MicrovoxelOverride,PlacedMarker,ObjectState,GeneratedTombstone,EnvironmentalLocal};
struct SpatialAddress{SpatialDomain domain{};std::uint64_t worldStableId{},spaceStableId{};std::int64_t x{},y{},z{};std::uint32_t subdomain{};auto operator<=>(const SpatialAddress&)const=default;};
struct SpatialDelta{SpatialAddress address;SpatialDeltaKind kind{};std::uint64_t stableObjectId{},contentId{},revision{1};std::vector<std::uint8_t>payload;bool operator==(const SpatialDelta&)const=default;};
struct SpatialDeltaBatch{std::uint32_t schemaVersion{1};std::vector<SpatialDelta>deltas;bool operator==(const SpatialDeltaBatch&)const=default;};
enum class DeltaCodecFailure:std::uint8_t{None,InvalidAddress,InvalidDelta,UnsupportedVersion,Malformed};
class SpatialDeltaCodec{public:[[nodiscard]]DeltaCodecFailure validate(const SpatialDelta&)const;[[nodiscard]]std::vector<std::uint8_t>encode(const SpatialDeltaBatch&)const;[[nodiscard]]DeltaCodecFailure decode(const std::vector<std::uint8_t>&,SpatialDeltaBatch&)const;};
class SpatialDeltaStore{public:DeltaCodecFailure apply(SpatialDelta);[[nodiscard]]std::vector<SpatialDelta>forAddress(const SpatialAddress&)const;[[nodiscard]]bool empty()const{return rows_.empty();}private:struct Key{SpatialAddress address;SpatialDeltaKind kind{};std::uint64_t objectId{};auto operator<=>(const Key&)const=default;};std::map<Key,SpatialDelta>rows_;};
} // namespace elysium::save
