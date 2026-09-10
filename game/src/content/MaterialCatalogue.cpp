// Intended function: centralize safe material calculations so systems do not invent incompatible constants.
#include "content/MaterialCatalogue.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
#include <cmath>
namespace elysium{
MaterialDefinition MaterialCatalogue::sanitize(MaterialDefinition d){
 auto nn=[](float v){return safe::nonNegative(v);};
 d.density=nn(d.density);d.hardness=nn(d.hardness);d.compressiveStrength=nn(d.compressiveStrength);d.tensileStrength=nn(d.tensileStrength);d.thermalConductivity=nn(d.thermalConductivity);d.heatCapacity=nn(d.heatCapacity);d.meltingPoint=nn(d.meltingPoint);d.electricalConductivity=nn(d.electricalConductivity);d.corrosionResistance=safe::finiteClamp(d.corrosionResistance,0.f,1.f);d.radiationShielding=nn(d.radiationShielding);d.fractureToughness=nn(d.fractureToughness);d.ignitionTemperature=nn(d.ignitionTemperature);d.thermalExpansion=nn(d.thermalExpansion);d.porosity=safe::finiteClamp(d.porosity,0.f,1.f);d.permeability=safe::finiteClamp(d.permeability,0.f,1.f);return d;}
bool MaterialCatalogue::add(MaterialDefinition d){if(!d.materialId||find(d.materialId))return false;defs_.push_back(sanitize(std::move(d)));std::sort(defs_.begin(),defs_.end(),[](auto&a,auto&b){return a.materialId<b.materialId;});return true;}
const MaterialDefinition*MaterialCatalogue::find(std::uint32_t id)const{auto i=std::lower_bound(defs_.begin(),defs_.end(),id,[](auto&a,std::uint32_t b){return a.materialId<b;});return i!=defs_.end()&&i->materialId==id?&*i:nullptr;}
float MaterialCatalogue::supportSpan(std::uint32_t id,float scale)const{auto*d=find(id);if(!d||!d->structural)return 0;return safe::nonNegative(std::sqrt(std::max(0.f,d->tensileStrength)/std::max(1.f,d->density))*std::max(.1f,safe::nonNegative(scale)));}
float MaterialCatalogue::miningWork(std::uint32_t id,float p)const{auto*d=find(id);return d?safe::nonNegative(std::max(.01f,d->hardness)/std::max(.01f,safe::nonNegative(p))):0;}
float MaterialCatalogue::thermalResponse(std::uint32_t id,float energy)const{auto*d=find(id);if(!d)return 0;return safe::nonNegative(safe::nonNegative(energy)/std::max(.01f,d->heatCapacity));}
float MaterialCatalogue::corrosionDamage(std::uint32_t id,float acid)const{auto*d=find(id);if(!d)return 0;return safe::nonNegative(safe::nonNegative(acid)*(1.f-safe::finiteClamp(d->corrosionResistance,0.f,1.f)));}
}
