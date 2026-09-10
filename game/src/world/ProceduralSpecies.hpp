// Intended function: deterministic fauna species grammar producing body plan, locomotion, diet, defenses, social traits and visual parameters from a planet seed.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
enum class FaunaBodyPlan:std::uint8_t{Biped,Quadruped,Hexapod,Serpentine,Avian,Aquatic};
enum class FaunaDiet:std::uint8_t{Herbivore,Omnivore,Carnivore,Filter,Scavenger,Chemovore};
struct ProceduralSpecies{std::uint64_t speciesId{};FaunaBodyPlan bodyPlan{};FaunaDiet diet{};float size{},mass{},speed{},armor{},aggression{},curiosity{},sociality{},heatTolerance{},coldTolerance{};std::uint8_t limbPairs{},colorHue{},pattern{};bool flying{},burrowing{},amphibious{},venomous{},bioluminescent{};};
ProceduralSpecies generateSpecies(std::uint64_t planetSeed,std::uint32_t speciesOrdinal,float gravity,float temperature,float oceanFraction);
std::vector<ProceduralSpecies>generateSpeciesSet(std::uint64_t planetSeed,std::uint32_t count,float gravity,float temperature,float oceanFraction);
}
