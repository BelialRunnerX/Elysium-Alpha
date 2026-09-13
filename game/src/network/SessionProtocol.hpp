#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Define future connection, handshake, content-version, world-generation, authentication, and disconnect message contracts.
struct SessionProtocolInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct SessionProtocolSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class SessionProtocolModel {
public:
 bool update(const SessionProtocolInput& input);
 const SessionProtocolSnapshot* get(std::uint64_t keyId) const;
 std::vector<SessionProtocolSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,SessionProtocolSnapshot> data_;
};

}
