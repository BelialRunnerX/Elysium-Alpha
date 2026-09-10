#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent hold/toggle conversion, timing assistance, aim assistance, repeated-input reduction, and control simplification.
struct InputAssistModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct InputAssistModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class InputAssistModelStore { public: bool apply(const InputAssistModelOp&); bool erase(std::uint64_t); const InputAssistModelData* find(std::uint64_t) const; std::vector<InputAssistModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,InputAssistModelData> data_; };
}
