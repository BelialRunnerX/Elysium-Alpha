#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Build patients, injuries, diseases, triage, treatments, staff, beds, supplies, quarantine, and alerts projections.
struct MedicalScreenModelOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct MedicalScreenModelData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class MedicalScreenModelStore { public: bool apply(const MedicalScreenModelOp&); bool erase(std::uint64_t); const MedicalScreenModelData* find(std::uint64_t) const; std::vector<MedicalScreenModelData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,MedicalScreenModelData> data_; };
}
