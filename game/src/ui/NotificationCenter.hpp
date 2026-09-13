#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Aggregate bounded player notifications by severity, category, stable subject, deduplication, acknowledgement, and expiry.
struct NotificationCenterInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct NotificationCenterSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class NotificationCenterModel {
public:
 bool update(const NotificationCenterInput& input);
 const NotificationCenterSnapshot* get(std::uint64_t keyId) const;
 std::vector<NotificationCenterSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,NotificationCenterSnapshot> data_;
};

}
