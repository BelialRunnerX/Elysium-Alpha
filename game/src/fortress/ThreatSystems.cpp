#include "fortress/Systems.hpp"

#include <array>
#include <algorithm>

namespace elysium::fortress {
namespace {
constexpr std::array<std::string_view, 12> kTraits{
    "segmented_armor", "phase_limb", "void_sac", "mirror_carapace", "neural_spines", "gravity_bladder",
    "regenerative_filament", "plasma_gland", "burrowing_claws", "sensor_crown", "spore_mantle", "pressure_shell"
};
constexpr std::array<std::string_view, 10> kAttacks{
    "breach_charge", "phase_lunge", "plasma_spray", "neural_scream", "gravity_pulse", "corrosive_spit",
    "kinetic_ram", "spawn_shard", "void_beam", "structure_devour"
};
constexpr std::array<std::string_view, 8> kSyndromes{
    "rift_fever", "glass_lung", "memory_bleed", "phase_rot", "neural_bloom", "void_burn", "gravity_sickness", "aetheric_echo"
};
}

RiftHorrorState generateRiftHorror(std::uint64_t seed, std::uint64_t epoch, SiteId target) {
    RiftHorrorState horror{};
    horror.id = StableId{makeDerivedId<StableId>(seed, target.value, 0x52494654484F5252ULL, epoch).value};
    const std::uint64_t h = deterministicToken(seed, target.value, 0x484F52524F52ULL, epoch);
    horror.generatedName = "Rift-" + std::to_string((h >> 16U) & 0xFFFFU) + "-" + std::to_string(h & 0xFFFU);
    horror.scale = 1.5f + static_cast<float>((h >> 24U) & 0xFFU) / 64.0f;
    horror.intelligence = static_cast<float>((h >> 40U) & 0xFFU) / 255.0f;
    horror.instability = static_cast<float>((h >> 48U) & 0xFFU) / 255.0f;
    for (std::size_t i = 0; i < 4; ++i) {
        horror.bodyTraits.push_back(ContentId{"elysium:rift/trait/" + std::string(kTraits[(h >> (i * 5U)) % kTraits.size()])});
    }
    for (std::size_t i = 0; i < 3; ++i) {
        horror.attacks.push_back(ContentId{"elysium:rift/attack/" + std::string(kAttacks[(h >> (12U + i * 6U)) % kAttacks.size()])});
    }
    horror.syndromes.push_back(ContentId{"elysium:syndrome/" + std::string(kSyndromes[(h >> 32U) % kSyndromes.size()])});
    horror.immunities.push_back(ContentId{"elysium:damage/rift"});
    if ((h & 1U) != 0U) horror.immunities.push_back(ContentId{"elysium:damage/void"});
    return horror;
}

void advanceThreat(ThreatState& threat, float fortressStrength, float days) {
    if (!threat.active || threat.defeated) return;
    const float d = std::max(0.0f, days);
    threat.escalation = saturate(threat.escalation + d * 0.08f * std::max(0.25f, threat.strength));
    const float defenseRatio = std::max(0.0f, fortressStrength) / std::max(0.1f, threat.strength);
    if (defenseRatio > 1.4f) {
        threat.strength = std::max(0.0f, threat.strength - d * defenseRatio * 0.12f);
    } else {
        threat.strength += d * (1.0f - defenseRatio) * 0.05f;
    }
    threat.defeated = threat.strength <= 0.05f;
    if (threat.defeated) threat.active = false;
}

void advanceSyndrome(SyndromeState& syndrome, float days, float treatment, float immunity) {
    const float d = std::max(0.0f, days);
    syndrome.incubation = std::max(0.0f, syndrome.incubation - d);
    if (syndrome.incubation > 0.0f) return;
    const float control = saturate(treatment) * 0.55f + saturate(immunity) * 0.45f;
    syndrome.progress = saturate(syndrome.progress + d * syndrome.severity * 0.08f * (1.0f - control) - d * control * 0.03f);
    syndrome.severity = saturate(syndrome.severity - d * saturate(treatment) * 0.02f);
}

float transmissionRisk(const SyndromeState& syndrome, float contactIntensity, float filtration,
                       float protectiveEquipment) {
    if (syndrome.quarantined) contactIntensity *= 0.15f;
    return saturate(syndrome.contagiousness * saturate(contactIntensity) *
                    (1.0f - saturate(filtration) * 0.65f) * (1.0f - saturate(protectiveEquipment) * 0.8f));
}

} // namespace elysium::fortress
