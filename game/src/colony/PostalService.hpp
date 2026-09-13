#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
namespace elysium {
// Intended function: Track local/intersettlement message and parcel demand, routing, delay, loss, and service capacity.
struct PostalServiceOp { std::uint64_t id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; };
struct PostalServiceData { std::uint64_t revision{0}, id{0}, owner{0}, ref{0}; double value{0.0}; std::uint32_t flags{0}; bool live{false}; };
class PostalServiceStore { public: bool apply(const PostalServiceOp&); bool erase(std::uint64_t); const PostalServiceData* find(std::uint64_t) const; std::vector<PostalServiceData> snapshot() const; void clear(); private: std::uint64_t revision_{1}; std::unordered_map<std::uint64_t,PostalServiceData> data_; };
}
