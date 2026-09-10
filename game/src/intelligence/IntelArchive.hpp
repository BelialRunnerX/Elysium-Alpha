#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Persist bounded intelligence records with age, provenance, confidence, classification, and supersession links.
struct IntelArchiveOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct IntelArchiveData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class IntelArchiveStore { public: bool apply(const IntelArchiveOp&); bool erase(std::uint64_t); const IntelArchiveData* find(std::uint64_t) const; std::vector<IntelArchiveData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,IntelArchiveData> data_; };
}
