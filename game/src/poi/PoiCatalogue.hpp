#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::poi {
enum class PoiLocationDomain:std::uint8_t{Surface,Subsurface,Orbit,Space,Anomalous};
enum class PoiPersistenceClass:std::uint8_t{RegenerableBaseline,SparseTouchedState,AlwaysPersistent};
enum class PoiFaction:std::uint32_t{Neutral=1u<<0,Frontier=1u<<1,Unsworn=1u<<2,Imperial=1u<<3,Ancient=1u<<4,Player=1u<<5,Hostile=1u<<6};
constexpr std::uint32_t poiFaction(PoiFaction f)noexcept{return static_cast<std::uint32_t>(f);}
struct PoiDef{
 std::uint64_t contentId{};PoiLocationDomain domain{};std::uint32_t factionMask{};std::uint64_t silhouetteFamilyContentId{},traversalHookContentId{},rewardDomainContentId{};PoiPersistenceClass persistenceClass{PoiPersistenceClass::RegenerableBaseline};std::vector<std::uint64_t>serviceCapabilityContentIds;std::uint64_t standingAdapterContentId{},knowledgeAdapterContentId{},industryAdapterContentId{},approachDecisionContentId{},laterOwnerContentId{};
};
enum class PoiValidationError:std::uint8_t{None,InvalidContentId,InvalidFaction,MissingSilhouette,MissingPersistenceIntent,Behaviorless,MissingDecisionValue,MissingOwner,DuplicateId};
struct PoiEligibility{PoiLocationDomain domain{};std::uint32_t acceptedFactionMask{};std::vector<std::uint64_t>requiredCapabilities;};
class PoiCatalogue{public: PoiValidationError publish(PoiDef);[[nodiscard]]const PoiDef*find(std::uint64_t)const;[[nodiscard]]std::vector<PoiDef>ordered()const;[[nodiscard]]std::vector<std::uint64_t>eligible(const PoiEligibility&)const;private:std::map<std::uint64_t,PoiDef>defs_;};
} // namespace elysium::poi
