#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
namespace elysium::content {
using ContentId=std::uint64_t;
enum class TemplateKind:std::uint8_t{Creature,Poi,Machine};enum class Workflow:std::uint8_t{Template,RegistryDraft,PresentationPass,ClosureValidation,IntegrationReady};
struct CreatureTemplate{ContentId id{},faction{},bodyPlan{},ability{},telegraph{},dropTable{},spawnSignal{},aiProfile{};double silhouetteMeters{},healthBudget{},damageBudget{},speedBudget{};bool farSleepIdentity{};};
struct PoiTemplate{ContentId id{},approachReason{},silhouette{},entryState{},traversalHook{},rewardDomain{},persistenceSchema{},faction{};std::vector<ContentId>variationModules;bool stableIdSlot{},tombstoneSlot{};};
struct MachineTemplate{ContentId id{};std::uint32_t tier{};std::vector<ContentId>inputs,outputs;ContentId powerRequirement{},environmentRequirement{},operatorRequirement{},automationSchema{},persistenceSchema{},presentationSchema{};std::string painAnswered,readableFailure;};
struct TemplateRecord{TemplateKind kind{};Workflow workflow{Workflow::Template};std::uint32_t schemaVersion{1};CreatureTemplate creature{};PoiTemplate poi{};MachineTemplate machine{};};
struct Validation{ContentId id{};std::string code;};
class TemplateGate{public:void addKnown(ContentId);bool add(TemplateRecord);bool advance(ContentId,Workflow);std::vector<Validation>validate(ContentId)const;std::vector<Validation>validateAll()const;bool freeze();private:std::set<ContentId>known_;std::map<ContentId,TemplateRecord>records_;bool frozen_{};ContentId idOf(const TemplateRecord&)const;};
}
