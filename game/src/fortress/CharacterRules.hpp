#pragma once

#include "fortress/Common.hpp"

#include <array>
#include <span>
#include <string_view>

namespace elysium::fortress {

enum class Element : std::uint8_t { Inert, Void, Plasma, Neural, Dimensional, Kinetic };
enum class Stat : std::uint8_t { Vitality, Fortitude, Resilience, Strength, Agility, Accuracy, Reflexes, Retribution, Intellect, Willpower, Luck, Presence };

enum class Race : std::uint8_t { Imperial, Druun, Veylari, Korrath, Lumari, Unsworn };
enum class CharacterClass : std::uint8_t { Medicae, Factor, Artificer, Enforcer, Psion, Voidrunner, Reclaimer, Warden, Marksman };

struct CharacterStats {
    std::array<float, 12> values{};
    float& operator[](Stat stat) { return values[static_cast<std::size_t>(stat)]; }
    const float& operator[](Stat stat) const { return values[static_cast<std::size_t>(stat)]; }
};

struct CharacterBuild {
    StableId id{};
    Race race{Race::Unsworn};
    CharacterClass characterClass{CharacterClass::Reclaimer};
    std::uint32_t level{1};
    float experience{};
    std::uint32_t unspentStatPoints{};
    CharacterStats stats{};
};

struct DamageContext {
    float baseDamage{};
    Element attackElement{Element::Inert};
    Element defenseElement{Element::Inert};
    bool melee{};
    bool criticalEligible{true};
    float attackerStrength{};
    float attackerAccuracy{};
    float defenderResilience{};
    float defenderFortitude{};
    float defenderReflexes{};
    float criticalMultiplier{1.5f};
    float independentMitigation{};
};

struct DamageResult {
    bool dodged{};
    bool critical{};
    float preMitigation{};
    float finalDamage{};
    float reflected{};
};

std::uint32_t experienceToNextLevel(std::uint32_t level);
float combineIndependentMitigation(std::span<const float> protections);
float elementAdvantage(Element attack, Element defense);
DamageResult resolveCharacterDamage(const DamageContext& context, std::uint64_t seed,
                                    std::uint64_t eventIdentity);

} // namespace elysium::fortress
