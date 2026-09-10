#pragma once
#include "content/BlockSchema.hpp"
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
namespace elysium::content {
enum class MachineDomain:std::uint8_t{Power,Industry,Logistics,Habitat,Defense,Command};enum class TickPolicy:std::uint8_t{EventDriven,FixedInterval,Sleeping};
struct PortDef{ContentId id{},filter{};bool input{},output{};std::uint64_t capacity{};};
struct EnvironmentRequirement{ContentId id{};double minimum{},maximum{};};
struct MachineSchema{ContentId id{},persistenceSchema{};std::uint32_t tier{};MachineDomain domain{MachineDomain::Industry};double powerGeneration{},powerDraw{};std::uint32_t brownoutPriority{};bool priorityDeclared{};TickPolicy tickPolicy{TickPolicy::EventDriven};std::uint32_t fixedIntervalTicks{};std::vector<PortDef>ports;std::vector<ContentId>processRefs;std::vector<EnvironmentRequirement>environment;std::vector<ContentId>automationInputs,automationOutputs;std::set<std::string>visualStates;};
struct MachineValidation{ContentId id{};std::string code;};
class MachineSchemaRegistry{public:bool addKnownReference(ContentId);bool add(MachineSchema);std::vector<MachineValidation>validate()const;bool freeze();const MachineSchema*find(ContentId)const;private:std::set<ContentId>refs_;std::map<ContentId,MachineSchema>defs_;bool frozen_{};};
struct AuthoringQuery{enum class Kind:std::uint8_t{Missing,Block,Machine};Kind kind{Kind::Missing};ContentId id{};std::uint32_t tier{};bool persistentObject{};};
class AuthoringFacade{public:AuthoringFacade(const BlockSchemaRegistry&,const MachineSchemaRegistry&);AuthoringQuery queryBlock(ContentId)const;AuthoringQuery queryMachine(ContentId)const;private:const BlockSchemaRegistry&blocks_;const MachineSchemaRegistry&machines_;};
}
