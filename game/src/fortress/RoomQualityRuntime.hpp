#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct RoomPolicy{StableId id{};ContentId type{};StableId owner{};std::vector<ContentId>tags;double minPrivacy{},maxNoise{1},targetComfort{.5};};struct RoomQualityInputs{double area{},materialValue{},craftsmanship{},cleanliness{},decor{},privacy{},noise{},comfort{},safety{},view{},culture{},personality{};};struct Contribution{std::string name;double value;};struct RoomQuality{double total{};std::vector<Contribution>contributions;};class RoomQualityRuntime{public:bool upsert(RoomPolicy);RoomQuality evaluate(StableId,const RoomQualityInputs&)const;private:std::unordered_map<StableId,RoomPolicy>rooms_;};}