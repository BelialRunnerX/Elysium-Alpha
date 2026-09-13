#pragma once
#include <array>
#include <cstdint>
#include <vector>
namespace elysium::anomaly {
using StableId=std::uint64_t; using ContentId=std::uint64_t;
struct Region{StableId regionId{};ContentId biomeId{};std::uint64_t spatialKey{};std::uint32_t priority{};double gravityScale{1},gravityBiasX{},gravityBiasY{},gravityBiasZ{},traversalScale{1},scannerReliability{1},scannerContradiction{},hazardScale{1};bool stateful{};};
struct Query{double gravityScale{1},gravityBiasX{},gravityBiasY{},gravityBiasZ{},traversalScale{1},scannerReliability{1},scannerContradiction{},hazardScale{1};std::vector<StableId>activeRegions;};
struct BoundaryEvent{StableId regionId{};bool entered{};};
Region generatedRegion(std::uint64_t universeSeed,ContentId biomeId,std::uint64_t spatialKey,std::uint32_t ordinal,std::uint32_t workerCount=1) noexcept;
Query evaluate(std::vector<Region> overlaps);
std::vector<BoundaryEvent> boundaryEvents(const std::vector<StableId>&before,const std::vector<StableId>&after);
bool untouchedRequiresSave(const Region&) noexcept;
}
