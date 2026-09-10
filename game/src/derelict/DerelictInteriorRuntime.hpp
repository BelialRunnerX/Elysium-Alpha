#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::derelict {
enum class SectionRole:std::uint8_t{CargoSpine,Habitation,Engineering,Bridge,Breach,Utility};
enum class ObjectKind:std::uint8_t{Door,Container,Breaker,Security,Seal,Breach,Console};
enum ObjectStateFlag:std::uint32_t{Opened=1u<<0,Looted=1u<<1,Destroyed=1u<<2,Restored=1u<<3,Sealed=1u<<4,Offline=1u<<5};
struct SectionDef{std::uint64_t contentId{};SectionRole role{};std::uint64_t capabilityMask{};std::uint32_t weight{1};};
struct ObjectDef{std::uint64_t contentId{};ObjectKind kind{};std::uint32_t defaultFlags{};};
struct DerelictKitDef{std::uint64_t contentId{},generatorVersion{1};std::vector<SectionDef>sections;std::vector<ObjectDef>objects;};
struct SectionRecord{std::uint64_t stableId{},contentId{};SectionRole role{};std::uint64_t capabilityMask{};std::uint32_t ordinal{};bool damaged{},offline{};bool operator==(const SectionRecord&)const=default;};
struct SectionLink{std::uint64_t stableId{},aSectionId{},bSectionId{};bool airlock{};bool operator==(const SectionLink&)const=default;};
struct ObjectRecord{std::uint64_t stableId{},sectionStableId{},contentId{};ObjectKind kind{};std::uint32_t flags{};bool operator==(const ObjectRecord&)const=default;};
struct ObjectDelta{std::uint64_t objectStableId{},revision{1};std::uint32_t setFlags{},clearFlags{};bool tombstoned{};};
struct DerelictInterior{std::uint64_t siteStableId{},kitContentId{},generatorVersion{};std::vector<SectionRecord>sections;std::vector<SectionLink>links;std::vector<ObjectRecord>objects;bool operator==(const DerelictInterior&)const=default;};
struct DerelictPersistentState{std::vector<ObjectDelta>objectDeltas;};
class DerelictInteriorRuntime{public:[[nodiscard]]DerelictInterior generate(std::uint64_t siteStableId,std::uint64_t worldSeed,const DerelictKitDef&,const DerelictPersistentState& persistentState = {})const;[[nodiscard]]std::vector<SectionRecord>sectionsWithCapability(const DerelictInterior&,std::uint64_t mask)const;[[nodiscard]]const ObjectRecord*findObject(const DerelictInterior&,std::uint64_t)const;bool applyDelta(DerelictPersistentState&,ObjectDelta)const;};
} // namespace elysium::derelict
