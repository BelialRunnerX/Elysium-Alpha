#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Build Chronicle, artifacts, wars, leaders, discoveries, settlements, and personal legacy projections.
struct HistoryScreenModelInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct HistoryScreenModelSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class HistoryScreenModelModel {
public:
 bool update(const HistoryScreenModelInput& input);
 const HistoryScreenModelSnapshot* get(std::uint64_t keyId) const;
 std::vector<HistoryScreenModelSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,HistoryScreenModelSnapshot> data_;
};

}
