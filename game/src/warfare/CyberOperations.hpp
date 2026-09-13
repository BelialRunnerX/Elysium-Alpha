#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track network intrusion, malware incidents, defensive posture, compromised systems, and recovery intents.
struct CyberOperationsOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct CyberOperationsData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class CyberOperationsStore { public: bool apply(const CyberOperationsOp&); bool erase(std::uint64_t); const CyberOperationsData* find(std::uint64_t) const; std::vector<CyberOperationsData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,CyberOperationsData> data_; };
}
