#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::streaming { using Address=std::uint64_t; enum class Lod:std::uint8_t{Editable,Coarse,Orbit,System}; struct ResidencyRequest{Address address{};Lod lod{};std::uint64_t viewerRevision{};double priority{};bool interactionProxy{false};}; struct ResidentHandle{Address address{};Lod lod{};std::uint64_t generation{};}; struct Candidate{ResidencyRequest request;std::uint64_t generation{};bool cancelled{false};}; class ResidencyAuthority{public:void submit(ResidencyRequest);std::vector<ResidencyRequest> orderedQueue()const;std::optional<ResidentHandle> publish(Candidate);void cancelBefore(std::uint64_t revision);void unload(Address);std::optional<ResidentHandle> resident(Address)const;std::size_t residentCount(Lod)const;private:std::vector<ResidencyRequest> queue_;std::unordered_map<Address,ResidentHandle> resident_;std::uint64_t minRevision_{0};}; }