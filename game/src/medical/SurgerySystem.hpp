#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent surgery plans, required facilities, staff skill, anesthesia, blood, implants, risk, and outcomes.
struct SurgerySystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SurgerySystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SurgerySystemModel {
public:
 bool update(const SurgerySystemInput& input);
 const SurgerySystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<SurgerySystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SurgerySystemSnapshot> data_;
};

}
