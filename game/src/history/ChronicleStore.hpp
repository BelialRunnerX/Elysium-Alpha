#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::history {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct ChronicleEvent{StableId id{};ContentId type{};std::uint32_t version{1};std::uint64_t epoch{};std::vector<StableId>participants,sites,items,factions;StableId source{},payload{},supersedes{};std::uint8_t significance{};bool visible{true};};class ChronicleStore{public:bool append(ChronicleEvent,std::string&);std::vector<ChronicleEvent>byEntity(StableId)const;std::vector<ChronicleEvent>visibleTimeline()const;std::size_t compactLowSignificance(std::uint8_t below);private:std::vector<ChronicleEvent>events_;std::unordered_map<StableId,std::size_t>byId_;};}