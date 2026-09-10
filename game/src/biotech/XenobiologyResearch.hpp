#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track alien organism studies, samples, containment, discoveries, risks, and technology unlock hooks.
struct XenobiologyResearchOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct XenobiologyResearchData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class XenobiologyResearchStore { public: bool apply(const XenobiologyResearchOp&); bool erase(std::uint64_t); const XenobiologyResearchData* find(std::uint64_t) const; std::vector<XenobiologyResearchData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,XenobiologyResearchData> data_; };
}
