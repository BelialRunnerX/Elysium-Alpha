// Intended function: shared material physics/gameplay catalogue for structural, mining, thermal, electrical and chemical behavior.
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace elysium{
enum class MaterialOrigin:std::uint8_t{Natural,Refined,Alloy,Composite,Synthetic,Exotic};
struct MaterialDefinition{
 std::uint32_t materialId{};std::string name;
 float density{},hardness{},compressiveStrength{},tensileStrength{},thermalConductivity{},heatCapacity{},meltingPoint{},electricalConductivity{},corrosionResistance{},radiationShielding{};
 std::uint32_t tagMask{};
 MaterialOrigin origin{MaterialOrigin::Natural};
 float fractureToughness{},ignitionTemperature{},thermalExpansion{},porosity{},permeability{};
 bool structural{},conductive{},flammable{},fluidReactive{};
};
class MaterialCatalogue{public:
 bool add(MaterialDefinition d);const MaterialDefinition*find(std::uint32_t id)const;
 float supportSpan(std::uint32_t id,float sectionScale=1)const;
 float miningWork(std::uint32_t id,float toolPower)const;
 float thermalResponse(std::uint32_t id,float energy)const;
 float corrosionDamage(std::uint32_t id,float acidStrength)const;
 const std::vector<MaterialDefinition>&all()const noexcept{return defs_;}
private:static MaterialDefinition sanitize(MaterialDefinition d);std::vector<MaterialDefinition>defs_;};
}
