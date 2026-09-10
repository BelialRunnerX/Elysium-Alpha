#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent localized wounds, fractures, burns, bleeding, organ trauma, pain, infection risk, and treatment progress.
struct InjurySystemInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct InjurySystemSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class InjurySystemModel {
public:
 bool update(const InjurySystemInput& input);
 const InjurySystemSnapshot* get(std::uint64_t keyId) const;
 std::vector<InjurySystemSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,InjurySystemSnapshot> data_;
};

}
