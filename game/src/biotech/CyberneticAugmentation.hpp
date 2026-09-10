#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track installed augmentations, body slots, power/data demands, maintenance, side effects, and equipment interaction.
struct CyberneticAugmentationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CyberneticAugmentationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CyberneticAugmentationStore { public: bool apply(const CyberneticAugmentationOp&); bool erase(std::uint64_t); const CyberneticAugmentationData* find(std::uint64_t) const; std::vector<CyberneticAugmentationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CyberneticAugmentationData> data_; };
}
