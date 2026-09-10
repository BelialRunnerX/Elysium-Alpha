#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace elysium::content {using ContentId=std::uint64_t;enum class IntegrationStatus:std::uint8_t{Proposed,Current};enum EvidenceBit:std::uint32_t{RecipeResolved=1u<<0,OwnerDeclared=1u<<1,PersistenceValid=1u<<2,UiAdapter=1u<<3,FocusedTests=1u<<4,TraceabilityUpdated=1u<<5,PoweredPriorityValid=1u<<6,Obtainable=1u<<7};struct MachineRecipeRow{ContentId machine{},constructionRecipe{},station{};std::uint32_t tier{},evidence{};IntegrationStatus status{IntegrationStatus::Proposed};};struct GateIssue{ContentId machine{};std::string code;};class MachineRecipeGate{public:bool addKnownMachine(ContentId);bool addRow(MachineRecipeRow);std::vector<GateIssue>validate()const;bool promote(ContentId);const MachineRecipeRow*find(ContentId)const;static constexpr std::uint32_t requiredEvidence=RecipeResolved|OwnerDeclared|PersistenceValid|UiAdapter|FocusedTests|TraceabilityUpdated|PoweredPriorityValid|Obtainable;private:std::map<ContentId,bool>machines_;std::map<ContentId,MachineRecipeRow>rows_;};}
