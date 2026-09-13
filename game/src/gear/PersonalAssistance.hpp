#pragma once
#include "survival/Survival.hpp"
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::gear {
enum class AssistanceReason { Ready, Unequipped, NoEnergy, InvalidRequest, Capacity, ForeignOwner, UnknownBuffer };
struct ExoskeletonSpec {
    float energyPerSecond{1}, carryBonus{}, throughputBonus{}, fallReduction{};
};
struct AssistanceQuery {
    AssistanceReason reason{AssistanceReason::Unequipped};
    float poweredFraction{}, carryBonus{}, throughputMultiplier{1}, fallReduction{}, energySpent{};
};
struct ExoskeletonState { std::uint64_t owner{}, item{}; bool equipped{}; ExoskeletonSpec spec; };
class PersonalExoskeleton {
public:
    bool restore(ExoskeletonState state);
    void unequip() { state_.equipped = false; }
    ExoskeletonState snapshot() const { return state_; }
    AssistanceQuery advance(float seconds, SurvivalState& body) const;
private: ExoskeletonState state_;
};
enum class PersonalBuffer : std::uint8_t { Cargo, ScannerCharge, ToolEnergy };
struct HarnessAccount {
    std::uint64_t owner{}, item{};
    PersonalBuffer kind{};
    std::uint64_t capacity{}, stored{};
};
struct BufferTransfer {
    AssistanceReason reason{AssistanceReason::InvalidRequest}; std::uint64_t moved{};
};
class PersonalHarness {
public:
    bool registerBuffer(HarnessAccount);
    BufferTransfer transfer(std::uint64_t actor, std::uint64_t from, std::uint64_t to,
                            PersonalBuffer kind, std::uint64_t amount);
    BufferTransfer consume(std::uint64_t actor, std::uint64_t item, PersonalBuffer, std::uint64_t amount);
    bool resize(std::uint64_t actor, std::uint64_t item, PersonalBuffer, std::uint64_t capacity);
    std::vector<HarnessAccount> snapshot() const;
    bool restore(const std::vector<HarnessAccount>&);
private:
    using Key = std::pair<std::uint64_t, PersonalBuffer>;
    std::map<Key,HarnessAccount> buffers_;
};
}
