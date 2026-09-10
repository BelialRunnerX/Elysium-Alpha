#pragma once
#include <algorithm>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::ai {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct AIIntent{ContentId type{};StableId target{};std::uint64_t decisionRevision{},sourceRevision{};ContentId reason{};};struct Stimulus{ContentId category{};StableId source{};double importance{};std::uint64_t tick{};};struct PathHandle{std::uint64_t requestId{},navRevision{};StableId target{};std::uint64_t targetAddress{};};struct ActorAIState{StableId actor{};AIIntent intent;std::vector<Stimulus>stimuli;std::optional<PathHandle>path;std::uint64_t dirtyFlags{};};class AIRuntimeScratch{public:bool upsert(ActorAIState);void sense(StableId,Stimulus,std::size_t maxPerCategory,std::uint64_t minTick);bool setPath(StableId,PathHandle,std::uint64_t currentNav,std::string&);bool invalidateIntent(StableId,std::uint64_t prerequisiteRevision);std::optional<ActorAIState>get(StableId)const;private:std::unordered_map<StableId,ActorAIState>rows_;};}