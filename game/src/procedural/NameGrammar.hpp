#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Generate deterministic multi-part names from culture-specific phoneme, syllable, title, and honorific grammars.
struct NameGrammarInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct NameGrammarSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class NameGrammarModel {
public:
 bool update(const NameGrammarInput& input);
 const NameGrammarSnapshot* get(std::uint64_t keyId) const;
 std::vector<NameGrammarSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,NameGrammarSnapshot> data_;
};

}
