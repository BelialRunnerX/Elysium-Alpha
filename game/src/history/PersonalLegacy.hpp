#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Summarize notable individual deeds and relationships for retirement, death, succession, statues, naming, and Chronicle use.
struct PersonalLegacyInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct PersonalLegacySnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class PersonalLegacyModel {
public:
 bool update(const PersonalLegacyInput& input);
 const PersonalLegacySnapshot* get(std::uint64_t keyId) const;
 std::vector<PersonalLegacySnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,PersonalLegacySnapshot> data_;
};

}
