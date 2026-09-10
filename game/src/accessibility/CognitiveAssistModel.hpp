#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent objective simplification, reminder cadence, UI density, terminology help, and guided-action preferences.
struct CognitiveAssistModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CognitiveAssistModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CognitiveAssistModelStore { public: bool apply(const CognitiveAssistModelOp&); bool erase(std::uint64_t); const CognitiveAssistModelData* find(std::uint64_t) const; std::vector<CognitiveAssistModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CognitiveAssistModelData> data_; };
}
