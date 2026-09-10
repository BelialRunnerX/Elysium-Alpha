#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Represent inheritable crop traits for yield, growth, climate tolerance, disease resistance, nutrition, and quality.
struct CropGeneticsInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct CropGeneticsSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class CropGeneticsModel {
public:
 bool update(const CropGeneticsInput& input);
 const CropGeneticsSnapshot* get(std::uint64_t keyId) const;
 std::vector<CropGeneticsSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,CropGeneticsSnapshot> data_;
};

}
