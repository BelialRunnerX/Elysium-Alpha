#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Expose curated read/event/intent surfaces to future sandbox scripts without direct access to simulation registries.
struct ScriptEventBusInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct ScriptEventBusSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class ScriptEventBusModel {
public:
 bool update(const ScriptEventBusInput& input);
 const ScriptEventBusSnapshot* get(std::uint64_t keyId) const;
 std::vector<ScriptEventBusSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,ScriptEventBusSnapshot> data_;
};

}
