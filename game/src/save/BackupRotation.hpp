#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Manage bounded backup generations and retention policy independently of authoritative save serialization.
struct BackupRotationOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct BackupRotationData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class BackupRotationStore { public: bool apply(const BackupRotationOp&); bool erase(std::uint64_t); const BackupRotationData* find(std::uint64_t) const; std::vector<BackupRotationData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,BackupRotationData> data_; };
}
