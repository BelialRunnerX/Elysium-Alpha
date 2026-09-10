#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <vector>
namespace elysium::world {using Cell=std::uint64_t;struct ThermalCell{Cell id{};double temperature{},oxygen{1},flammability{};bool burning{false};};struct FireEvent{Cell cell{};double heat{},smoke{},damage{};};class FireThermalRuntime{public:void set(ThermalCell);void ignite(Cell,double sourceHeat);std::vector<FireEvent>step(std::size_t budget);private:std::unordered_map<Cell,ThermalCell>cells_;std::deque<Cell>frontier_;};}