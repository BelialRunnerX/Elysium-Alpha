#include "ShipModuleProductionCatalogue.hpp"
#include <cmath>
namespace elysium::content {
bool ShipModuleProductionCatalogue::add(ShipModuleProductionRow r,std::string& why){ if(!r.id||!r.schemaVersion||!std::isfinite(r.mass)||r.mass<0||!std::isfinite(r.powerDraw)||r.powerDraw<0){why="invalid row";return false;} if(r.status!=AvailabilityStatus::Design && !r.repairRecipe){why="repair path required";return false;} if(!rows_.emplace(r.id,std::move(r)).second){why="duplicate module";return false;} return true; }
bool ShipModuleProductionCatalogue::validate(std::string& why) const { for(auto& [id,r]:rows_){ if(r.tier>64){why="impossible tier";return false;} if(r.status==AvailabilityStatus::Available && r.acquisitionRefs.empty()){why="available module unobtainable";return false;} if((r.slot==ShipSlot::Propulsion||r.slot==ShipSlot::Defense) && !r.hardpoint){why="hardpoint required";return false;} } return true; }
std::optional<ShipModuleProductionRow> ShipModuleProductionCatalogue::get(ContentId id) const { auto i=rows_.find(id); if(i==rows_.end())return std::nullopt; return i->second; }
std::vector<ContentId> ShipModuleProductionCatalogue::ids() const { std::vector<ContentId> v; for(auto& [id,_]:rows_)v.push_back(id); std::sort(v.begin(),v.end()); return v; }
}