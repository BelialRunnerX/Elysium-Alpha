#include "content/ShipModuleCatalogue.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Register stable ship modules, slots, stats, power/heat/fuel, crew, damage behavior, recipes, and visual hooks.
bool ShipModuleCatalogueModel::update(const ShipModuleCatalogueInput& input) { if(input.keyId==0) return false; auto& s=data_[input.keyId]; s.revision=revision_++; s.ownerId=input.ownerId; s.keyId=input.keyId; s.amount=input.amount; s.policy=input.policy; s.valid=true; return true; }
const ShipModuleCatalogueSnapshot* ShipModuleCatalogueModel::get(std::uint64_t keyId) const { auto it=data_.find(keyId); return it==data_.end()?nullptr:&it->second; }
std::vector<ShipModuleCatalogueSnapshot> ShipModuleCatalogueModel::ordered() const { std::vector<ShipModuleCatalogueSnapshot> out; out.reserve(data_.size()); for(const auto& [id,s]:data_) if(s.valid) out.push_back(s); std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.keyId<b.keyId;}); return out; }
void ShipModuleCatalogueModel::clear() { data_.clear(); revision_=1; }
}
