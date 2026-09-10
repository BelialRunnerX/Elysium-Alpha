#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Represent subtitle text, speaker, priority, sound direction, timing, non-speech cues, and readability options.
struct SubtitleModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct SubtitleModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class SubtitleModelStore { public: bool apply(const SubtitleModelOp&); bool erase(std::uint64_t); const SubtitleModelData* find(std::uint64_t) const; std::vector<SubtitleModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,SubtitleModelData> data_; };
}
