#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace elysium::agriculture {

enum class DomesticCapability : std::uint32_t {
    Grazer=1u<<0,Fowl=1u<<1,Woolbeast=1u<<2,Swine=1u<<3,PackBeast=1u<<4,Pollinator=1u<<5
};
constexpr std::uint32_t capability(DomesticCapability c) noexcept{return static_cast<std::uint32_t>(c);}
enum class ProductTrigger:std::uint8_t{Periodic,OnRetire};

struct DomesticProductDefinition{
    std::uint64_t productContentId{};
    ProductTrigger trigger{ProductTrigger::Periodic};
    std::uint64_t intervalTicks{};
    std::uint64_t quantity{1};
};
struct DomesticArchetypeDefinition{
    std::uint64_t contentId{};
    std::uint32_t capabilityMask{};
    double hungerRate{},thirstRate{},shelterNeed{};
    std::vector<DomesticProductDefinition> products;
};
struct DomesticNeeds{double hunger{},thirst{},shelter{},health{1.0};};
struct DomesticAnimalRecord{
    std::uint64_t stableId{},ownerId{},archetypeContentId{},nameRef{},wildReconstructionKey{},revision{1};
    DomesticNeeds needs{};
    std::vector<std::uint64_t> productProgressTicks;
    bool alive{true};
};
struct DomesticationTransaction{
    std::uint64_t transactionId{},sourceTransientActorId{},wildReconstructionKey{},newStableId{},ownerId{},archetypeContentId{},nameRef{};
};
struct DomesticCareInput{std::uint64_t stableId{};double feedCommit{},waterCommit{},shelterQuality{};bool roomAvailable{true};};
struct ProductIntent{
    std::uint64_t intentId{},animalStableId{},productContentId{},quantity{},revision{1};ProductTrigger trigger{ProductTrigger::Periodic};
};
struct DomesticSnapshot{
    std::vector<DomesticArchetypeDefinition> archetypes;
    std::vector<DomesticAnimalRecord> animals;
    std::vector<ProductIntent> pendingProducts;
    std::vector<std::uint64_t> consumedDomesticationTransactions;
};
class DomesticAnimalRuntime{
public:
 bool publish(DomesticArchetypeDefinition);
 bool promote(const DomesticationTransaction&);
 bool applyCare(const DomesticCareInput&);
 void step(std::uint64_t ticks);
 bool retire(std::uint64_t stableId,std::uint64_t transactionId);
 [[nodiscard]]const DomesticAnimalRecord*find(std::uint64_t stableId)const;
 [[nodiscard]]std::vector<ProductIntent>pendingProducts()const;
 bool acknowledgeProduct(std::uint64_t intentId);
 [[nodiscard]]DomesticSnapshot snapshot()const;
 bool restore(const DomesticSnapshot&);
private:
 void emitProduct(DomesticAnimalRecord&,const DomesticProductDefinition&,std::size_t productIndex,ProductTrigger);
 std::map<std::uint64_t,DomesticArchetypeDefinition>archetypes_;
 std::map<std::uint64_t,DomesticAnimalRecord>animals_;
 std::map<std::uint64_t,ProductIntent>pending_;
 std::map<std::uint64_t,bool>transactions_;
};
} // namespace elysium::agriculture
