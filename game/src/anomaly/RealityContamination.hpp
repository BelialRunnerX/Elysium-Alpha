#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Track persistent anomaly contamination on items, actors, chunks, machines, and research samples.
struct RealityContaminationInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct RealityContaminationSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class RealityContaminationModel {
public:
 bool update(const RealityContaminationInput& input);
 const RealityContaminationSnapshot* get(std::uint64_t keyId) const;
 std::vector<RealityContaminationSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,RealityContaminationSnapshot> data_;
};

}
