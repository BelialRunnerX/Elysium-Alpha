#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::travel {using StableId=std::uint64_t;struct RailEdge{StableId id{},from{},to{};double cost{1};bool enabled{true};};struct Cart{StableId id{};double mass{},capacity{},speed{};StableId owner{};};class CargoRailRuntime{public:bool addEdge(RailEdge);bool addCart(Cart);std::vector<StableId>route(StableId from,StableId to)const;private:std::unordered_map<StableId,RailEdge>edges_;std::unordered_map<StableId,Cart>carts_;};}