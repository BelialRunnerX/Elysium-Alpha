#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

namespace elysium {

// Intended function: Register stable quest templates, objective graphs, rewards, eligibility, history policy, and localization keys.
struct QuestCatalogueInput { std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; };
struct QuestCatalogueSnapshot { std::uint64_t revision{0}; std::uint64_t ownerId{0}; std::uint64_t keyId{0}; double amount{0.0}; std::uint32_t policy{0}; bool valid{false}; };
class QuestCatalogueModel {
public:
 bool update(const QuestCatalogueInput& input);
 const QuestCatalogueSnapshot* get(std::uint64_t keyId) const;
 std::vector<QuestCatalogueSnapshot> ordered() const;
 void clear();
private:
 std::uint64_t revision_{1};
 std::unordered_map<std::uint64_t,QuestCatalogueSnapshot> data_;
};

}
