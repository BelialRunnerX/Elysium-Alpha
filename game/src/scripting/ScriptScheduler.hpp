#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Schedule deterministic bounded script callbacks by simulation tick and stable script identity.
struct ScriptSchedulerInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ScriptSchedulerSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ScriptSchedulerModel {
public:
 bool update(const ScriptSchedulerInput& input);
 const ScriptSchedulerSnapshot* get(std::uint64_t keyId) const;
 std::vector<ScriptSchedulerSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ScriptSchedulerSnapshot> data_;
};

}
