#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::citizens {using ContentId=std::uint64_t;using StableId=std::uint64_t;struct NeedState{ContentId id{};double deficit{};double decayPerTick{};};struct Trait{ContentId id{};double weight{};};struct Memory{ContentId kind{};double valence{};double intensity{};std::uint64_t tick{};};struct MindState{StableId citizen{};std::vector<NeedState>needs;std::vector<Trait>traits,values,preferences;std::vector<Memory>memories;double focus{1.0};double stress{};std::uint8_t stressBand{};std::uint64_t crisisCooldownUntil{};};struct MindSnapshot{double focus{},stress{};std::uint8_t stressBand{};std::vector<std::pair<ContentId,double>>needDeficits;};class MindRuntime{public:bool add(MindState,std::string&);void advance(StableId,std::uint64_t ticks);bool satisfy(StableId,ContentId,double);MindSnapshot snapshot(StableId)const;bool shouldCrisis(StableId,std::uint64_t tick,double threshold)const;private:static double clamp01(double);std::unordered_map<StableId,MindState>rows_;};}