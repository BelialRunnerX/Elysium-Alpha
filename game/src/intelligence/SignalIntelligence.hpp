#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track intercepted communications, signal sources, confidence, decryption progress, and strategic intel outputs.
struct SignalIntelligenceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SignalIntelligenceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SignalIntelligenceStore { public: bool apply(const SignalIntelligenceOp&); bool erase(std::uint64_t); const SignalIntelligenceData* find(std::uint64_t) const; std::vector<SignalIntelligenceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SignalIntelligenceData> data_; };
}
