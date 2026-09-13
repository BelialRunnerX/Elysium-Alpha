// Intended function: imported world implementation for ThreatEcology; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/ThreatEcology.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr std::uint64_t kRiftRootLabel = 0x5249465452484F52ULL;      // RIFTRHOR
constexpr std::uint64_t kRiftStableLabel = 0x5248464947555245ULL;    // RHFIGURE
constexpr std::uint64_t kBodyLabel = 0x424F4459504C414EULL;          // BODYPLAN
constexpr std::uint64_t kTissueLabel = 0x5449535355450001ULL;
constexpr std::uint64_t kScaleLabel = 0x5343414C45000001ULL;
constexpr std::uint64_t kLocomotionLabel = 0x4C4F434F4D4F5401ULL;
constexpr std::uint64_t kDefenseLabel = 0x444546454E534501ULL;
constexpr std::uint64_t kAttackOneLabel = 0x41545441434B0001ULL;
constexpr std::uint64_t kAttackTwoLabel = 0x41545441434B0002ULL;
constexpr std::uint64_t kEmissionLabel = 0x454D495353494F4EULL;
constexpr std::uint64_t kVulnerabilityLabel = 0x56554C4E45524142ULL;
constexpr std::uint64_t kNameOneLabel = 0x4E414D4500000001ULL;
constexpr std::uint64_t kNameTwoLabel = 0x4E414D4500000002ULL;
constexpr std::uint64_t kHistoryLabel = 0x5448524541544556ULL;       // THREATEV

constexpr std::size_t kThreatFamilyCount = 9;
constexpr std::size_t kThreatLifecycleCount = 10;
constexpr std::size_t kBodyCount = 7;
constexpr std::size_t kTissueCount = 8;
constexpr std::size_t kLocomotionCount = 7;
constexpr std::size_t kDefenseCount = 6;
constexpr std::size_t kAttackCount = 9;
constexpr std::size_t kEmissionCount = 7;
constexpr std::size_t kVulnerabilityCount = 6;

std::uint64_t generatorRoot(std::uint64_t worldSeed,
                            std::uint64_t siteId,
                            std::uint64_t serial,
                            std::uint32_t version) {
    auto h = mix64(worldSeed ^ kRiftRootLabel);
    h = mix64(h ^ siteId);
    h = mix64(h ^ serial);
    h = mix64(h ^ static_cast<std::uint64_t>(version));
    return h;
}

std::size_t axisIndex(std::uint64_t root, std::uint64_t label, std::size_t count) {
    return count == 0 ? 0 : static_cast<std::size_t>(mix64(root ^ label) % count);
}

float axis01(std::uint64_t root, std::uint64_t label) {
    const auto h = mix64(root ^ label);
    return static_cast<float>((h >> 40U) & 0xFFFFFFU) / static_cast<float>(0xFFFFFFU);
}

bool finiteNonNegative(float value) {
    return std::isfinite(value) && value >= 0.0f;
}

bool finiteUnit(float value) {
    return std::isfinite(value) && value >= 0.0f && value <= 1.0f;
}

bool enumInRange(int value, int maxExclusive) {
    return value >= 0 && value < maxExclusive;
}

std::string makeRiftName(std::uint64_t root, RiftTissue tissue, RiftAttackVerb attack) {
    static constexpr std::array<const char*, 16> first{
        "Nhal", "Veyr", "Oss", "Khar", "Ilyr", "Sovar", "Asha", "Morn",
        "Teth", "Ruun", "Cael", "Zyra", "Draal", "Eid", "Vor", "Sere"
    };
    static constexpr std::array<const char*, 16> second{
        "ath", "ek", "ion", "ul", "ara", "os", "ith", "uun",
        "esh", "or", "yx", "ael", "orn", "is", "eth", "um"
    };
    const auto a = first[axisIndex(root, kNameOneLabel, first.size())];
    const auto b = second[axisIndex(root, kNameTwoLabel, second.size())];
    std::ostringstream out;
    out << a << b << " the " << riftTissueName(tissue) << ' ' << riftAttackVerbName(attack);
    return out.str();
}

std::string signatureFor(RiftAttackVerb attack, RiftEmission emission) {
    switch (attack) {
        case RiftAttackVerb::Breach:
            return std::string("It marks a structural seam, then commits to a telegraphed breach through ") + riftEmissionName(emission) + ".";
        case RiftAttackVerb::Grab:
            return "It pauses before a long-reach grab that isolates one target from nearby defenders.";
        case RiftAttackVerb::Spit:
            return std::string("Its throat/casting organ brightens before launching a persistent ") + riftEmissionName(emission) + " zone.";
        case RiftAttackVerb::Beam:
            return "A visible aiming line precedes a sweeping beam, creating a reliable cover-and-movement response.";
        case RiftAttackVerb::Cloud:
            return std::string("Local sensors spike before it vents a moving ") + riftEmissionName(emission) + " cloud.";
        case RiftAttackVerb::Charge:
            return "It anchors, lowers its profile, and then charges along a committed lane that can be redirected or trapped.";
        case RiftAttackVerb::SummonSpawn:
            return "It becomes briefly stationary while producing lesser spawn, creating a target-priority window.";
        case RiftAttackVerb::DrainPower:
            return "Lights, batteries, and powered defenses sag in a visible radius before it attacks the weakened network.";
        case RiftAttackVerb::CorruptMachines:
            return "Affected machinery emits a distinct fault state before remote controls invert or shut down.";
    }
    return "Its signature action is visible before the dangerous phase begins.";
}

std::string counterplayFor(RiftVulnerability vulnerability) {
    switch (vulnerability) {
        case RiftVulnerability::Material:
            return "Identify the tissue/material response and use the matching fracture or penetration tool profile.";
        case RiftVulnerability::Environment:
            return "Force the creature into an incompatible atmosphere, temperature, pressure, or contained hazard volume.";
        case RiftVulnerability::BodyRegion:
            return "Expose and focus the vulnerable body region instead of distributing damage across the whole silhouette.";
        case RiftVulnerability::ExposedPhase:
            return "Survive the signature cycle and concentrate damage during its recovery/exposed phase.";
        case RiftVulnerability::SoundSignal:
            return "Use sensor, sound, or signal countermeasures to force a readable vulnerable state.";
        case RiftVulnerability::PowerState:
            return "Cut, overload, or drain its power state so its defensive organ drops before committing heavy damage.";
    }
    return "Observe the signature behavior and exploit its documented counterplay state.";
}

std::uint64_t historyEventId(const ThreatRecord& record, ThreatHistoryEventKind kind) {
    auto h = mix64(record.stableId ^ kHistoryLabel);
    h = mix64(h ^ static_cast<std::uint64_t>(record.nextHistorySerial));
    h = mix64(h ^ static_cast<std::uint64_t>(kind));
    return h == 0 ? 1 : h;
}

ThreatHistoryEvent makeHistory(ThreatRecord& record,
                               ThreatHistoryEventKind kind,
                               std::uint64_t campaignTick,
                               std::string summary,
                               std::uint64_t relatedStableId = 0) {
    ThreatHistoryEvent event{};
    event.eventId = historyEventId(record, kind);
    event.threatStableId = record.stableId;
    event.siteId = record.siteId;
    event.campaignTick = campaignTick;
    event.kind = kind;
    event.relatedStableId = relatedStableId;
    event.summary = std::move(summary);
    ++record.nextHistorySerial;
    return event;
}

bool hasAttack(const ThreatRecord& threat, RiftAttackVerb attack) {
    return threat.riftHorror &&
           (threat.riftHorror->primaryAttack == attack || threat.riftHorror->secondaryAttack == attack);
}

ThreatObjectiveKind objectiveForNode(ThreatDefenseNodeKind kind) {
    switch (kind) {
        case ThreatDefenseNodeKind::Gate:
        case ThreatDefenseNodeKind::Wall:
        case ThreatDefenseNodeKind::Airlock:
            return ThreatObjectiveKind::Breach;
        case ThreatDefenseNodeKind::ExposedShaft:
        case ThreatDefenseNodeKind::Tunnel:
            return ThreatObjectiveKind::Infiltrate;
        case ThreatDefenseNodeKind::Utility:
            return ThreatObjectiveKind::DisableUtility;
        case ThreatDefenseNodeKind::Defense:
            return ThreatObjectiveKind::DisableDefense;
        case ThreatDefenseNodeKind::Population:
            return ThreatObjectiveKind::AttackPopulation;
        case ThreatDefenseNodeKind::Objective:
            return ThreatObjectiveKind::SeizeObjective;
        case ThreatDefenseNodeKind::Terrain:
            break;
    }
    
    return ThreatObjectiveKind::Approach;
}

float baseNodeValue(ThreatDefenseNodeKind kind) {
    switch (kind) {
        case ThreatDefenseNodeKind::Gate: return 8.0f;
        case ThreatDefenseNodeKind::Wall: return 5.0f;
        case ThreatDefenseNodeKind::Utility: return 10.0f;
        case ThreatDefenseNodeKind::ExposedShaft: return 9.0f;
        case ThreatDefenseNodeKind::Tunnel: return 8.0f;
        case ThreatDefenseNodeKind::Airlock: return 9.0f;
        case ThreatDefenseNodeKind::Terrain: return 2.0f;
        case ThreatDefenseNodeKind::Defense: return 8.0f;
        case ThreatDefenseNodeKind::Population: return 8.0f;
        case ThreatDefenseNodeKind::Objective: return 12.0f;
    }
    return 0.0f;
}

std::string planReason(ThreatObjectiveKind objective, const ThreatDefenseNode& node) {
    std::ostringstream out;
    switch (objective) {
        case ThreatObjectiveKind::Approach: out << "low-cost approach lane"; break;
        case ThreatObjectiveKind::Breach: out << "reachable structural breach point"; break;
        case ThreatObjectiveKind::Infiltrate: out << "exposed shaft/tunnel bypasses stronger perimeter"; break;
        case ThreatObjectiveKind::DisableUtility: out << "utility value can degrade life-support or power response"; break;
        case ThreatObjectiveKind::DisableDefense: out << "defense node suppresses follow-on attackers"; break;
        case ThreatObjectiveKind::AttackPopulation: out << "exposed population creates evacuation pressure"; break;
        case ThreatObjectiveKind::SeizeObjective: out << "high-value site objective"; break;
    }
    out << "; exposure=" << node.exposure << ", hardness=" << node.hardness
        << ", defense=" << node.defenseCoverage;
    return out.str();
}


} // namespace

const char* threatFamilyName(ThreatFamily value) {
    switch (value) {
        case ThreatFamily::WildlifePredator: return "Wildlife/Predator";
        case ThreatFamily::Raider: return "Raider/Unsworn Hostile";
        case ThreatFamily::ImperialEnforcement: return "Imperial Enforcement";
        case ThreatFamily::RiftHorror: return "Rift Horror";
        case ThreatFamily::Titan: return "Titan-Class Entity";
        case ThreatFamily::InfiltratorSyndrome: return "Infiltrator Syndrome";
        case ThreatFamily::MachineCorruption: return "Machine Corruption";
        case ThreatFamily::EnvironmentalCatastrophe: return "Environmental Catastrophe";
        case ThreatFamily::CivilUnrest: return "Civil Unrest";
    }
    return "Unknown Threat";
}

const char* threatLifecycleName(ThreatLifecycle value) {
    switch (value) {
        case ThreatLifecycle::Dormant: return "Dormant";
        case ThreatLifecycle::Signaled: return "Signaled";
        case ThreatLifecycle::Approaching: return "Approaching";
        case ThreatLifecycle::Engaged: return "Engaged";
        case ThreatLifecycle::Retreating: return "Retreating";
        case ThreatLifecycle::Migrating: return "Migrating";
        case ThreatLifecycle::Contained: return "Contained";
        case ThreatLifecycle::Defeated: return "Defeated";
        case ThreatLifecycle::Dead: return "Dead";
        case ThreatLifecycle::Resolved: return "Resolved";
    }
    return "Unknown";
}

const char* riftBodyPlanName(RiftBodyPlan value) {
    switch (value) {
        case RiftBodyPlan::Quadruped: return "Quadruped";
        case RiftBodyPlan::Serpentine: return "Serpentine";
        case RiftBodyPlan::Radial: return "Radial";
        case RiftBodyPlan::Flyer: return "Flyer";
        case RiftBodyPlan::Burrower: return "Burrower";
        case RiftBodyPlan::ManyLimbed: return "Many-Limbed";
        case RiftBodyPlan::Amorphous: return "Amorphous";
    }
    return "Unknown";
}

const char* riftTissueName(RiftTissue value) {
    switch (value) {
        case RiftTissue::Flesh: return "Flesh";
        case RiftTissue::Chitin: return "Chitin";
        case RiftTissue::Crystal: return "Crystal";
        case RiftTissue::Metal: return "Metal";
        case RiftTissue::Glass: return "Glass";
        case RiftTissue::PlasmaSheath: return "Plasma-Sheathed";
        case RiftTissue::Fungal: return "Fungal";
        case RiftTissue::VoidComposite: return "Void-Composite";
    }
    return "Unknown";
}

const char* riftScaleName(RiftScale value) {
    switch (value) {
        case RiftScale::LargePredator: return "Large Predator";
        case RiftScale::Siege: return "Fortress Threat";
        case RiftScale::Titan: return "Titan";
    }
    return "Unknown";
}

const char* riftLocomotionName(RiftLocomotion value) {
    switch (value) {
        case RiftLocomotion::Walk: return "Walk";
        case RiftLocomotion::Climb: return "Climb";
        case RiftLocomotion::Burrow: return "Burrow";
        case RiftLocomotion::Fly: return "Fly";
        case RiftLocomotion::PhaseStep: return "Phase-Step";
        case RiftLocomotion::Swim: return "Swim";
        case RiftLocomotion::WallCrawl: return "Wall-Crawl";
    }
    return "Unknown";
}

const char* riftDefenseName(RiftDefense value) {
    switch (value) {
        case RiftDefense::ArmorPlates: return "Armor Plates";
        case RiftDefense::Regeneration: return "Regeneration";
        case RiftDefense::ShieldOrgan: return "Shield Organ";
        case RiftDefense::Dispersal: return "Dispersal";
        case RiftDefense::Camouflage: return "Camouflage";
        case RiftDefense::ReactiveCarapace: return "Reactive Carapace";
    }
    return "Unknown";
}

const char* riftAttackVerbName(RiftAttackVerb value) {
    switch (value) {
        case RiftAttackVerb::Breach: return "Breacher";
        case RiftAttackVerb::Grab: return "Grasper";
        case RiftAttackVerb::Spit: return "Spitter";
        case RiftAttackVerb::Beam: return "Beamer";
        case RiftAttackVerb::Cloud: return "Cloudbearer";
        case RiftAttackVerb::Charge: return "Charger";
        case RiftAttackVerb::SummonSpawn: return "Broodcaller";
        case RiftAttackVerb::DrainPower: return "Power-Drainer";
        case RiftAttackVerb::CorruptMachines: return "Machine-Corrupter";
    }
    return "Unknown";
}

const char* riftEmissionName(RiftEmission value) {
    switch (value) {
        case RiftEmission::AcidMist: return "acid mist";
        case RiftEmission::Spores: return "spores";
        case RiftEmission::Radiation: return "radiation";
        case RiftEmission::NeuralEffect: return "neural interference";
        case RiftEmission::Heat: return "heat";
        case RiftEmission::CryogenicWake: return "cryogenic wake";
        case RiftEmission::DimensionalDistortion: return "dimensional distortion";
    }
    return "unknown emission";
}

const char* riftVulnerabilityName(RiftVulnerability value) {
    switch (value) {
        case RiftVulnerability::Material: return "Material";
        case RiftVulnerability::Environment: return "Environment";
        case RiftVulnerability::BodyRegion: return "Body Region";
        case RiftVulnerability::ExposedPhase: return "Exposed Phase";
        case RiftVulnerability::SoundSignal: return "Sound/Signal";
        case RiftVulnerability::PowerState: return "Power State";
    }
    return "Unknown";
}

ThreatSignalPolicy threatSignalPolicy(ThreatFamily family) {
    ThreatSignalPolicy out{};
    switch (family) {
        case ThreatFamily::WildlifePredator:
            out.minimumReadableWarningSeconds = 6.0f;
            out.signal = "Tracks, calls, disturbed fauna, or sensor motion build before contact.";
            out.counterplay = "Close livestock access, light/secure approaches, and prepare a local hunting or evacuation response.";
            break;
        case ThreatFamily::Raider:
            out.minimumReadableWarningSeconds = 20.0f;
            out.signal = "Scouts, missing supplies, perimeter pings, or distant engine signatures reveal the approach.";
            out.counterplay = "Secure stores and civilians, man chokepoints, and protect likely breach or theft objectives.";
            break;
        case ThreatFamily::ImperialEnforcement:
            out.minimumReadableWarningSeconds = 60.0f;
            out.signal = "The File announces enforcement and sensors identify an inbound Imperial action.";
            out.counterplay = "Use the announced window to repair the beacon perimeter, stock defenses, evacuate civilians, or leave the system.";
            break;
        case ThreatFamily::RiftHorror:
        case ThreatFamily::Titan:
            out.minimumReadableWarningSeconds = 30.0f;
            out.signal = "Anomalous sensor, ecological, and environmental signatures escalate before the named entity reaches the site.";
            out.counterplay = "Inspect the generated signature and vulnerability, then reshape defenses around that specific counterplay.";
            break;
        case ThreatFamily::InfiltratorSyndrome:
            out.minimumReadableWarningSeconds = 15.0f;
            out.signal = "Contradictory identity records, medical anomalies, access violations, or witness reports accumulate before overt transformation/sabotage.";
            out.counterplay = "Investigate evidence, restrict access, quarantine suspects, and avoid omniscient punishment without observations.";
            break;
        case ThreatFamily::MachineCorruption:
            out.minimumReadableWarningSeconds = 10.0f;
            out.signal = "Fault telemetry, command disagreement, power spikes, or corrupted automation states precede destructive behavior.";
            out.counterplay = "Isolate affected networks, switch to manual control, cut power where safe, and dispatch repair/security jobs.";
            break;
        case ThreatFamily::EnvironmentalCatastrophe:
            out.minimumReadableWarningSeconds = 8.0f;
            out.signal = "Sensors and local world state expose fire, flood, pressure, storm, reactor, radiation, or contamination escalation.";
            out.counterplay = "Use evacuation, isolation, suppression, pumping, sealing, filtration, or shutdown appropriate to the hazard.";
            break;
        case ThreatFamily::CivilUnrest:
            out.minimumReadableWarningSeconds = 30.0f;
            out.signal = "Stress, grievance, faction, justice, or governance indicators worsen before organized refusal, violence, or schism.";
            out.counterplay = "Inspect causes and address safety, justice, policy, leadership, or social needs before using security force.";
            break;
    }
    return out;
}

ThreatCapabilityProfile threatCapabilityProfile(const ThreatRecord& threat) {
    ThreatCapabilityProfile out{};
    switch (threat.family) {
        case ThreatFamily::WildlifePredator:
            out.siegeTier = 0; out.canAttackPopulation = true; break;
        case ThreatFamily::Raider:
            out.siegeTier = 2; out.canBreach = true; out.canInfiltrate = true; out.canDisableUtility = true;
            out.canDisableDefense = true; out.canAttackPopulation = true; out.canSeizeObjective = true;
            out.structuralPowerScale = 0.8f;
            break;
        case ThreatFamily::ImperialEnforcement:
            out.siegeTier = 3; out.canBreach = true; out.canDisableUtility = true; out.canDisableDefense = true;
            out.canAttackPopulation = true; out.canSeizeObjective = true; out.structuralPowerScale = 1.1f;
            break;
        case ThreatFamily::RiftHorror:
        case ThreatFamily::Titan:
            out.siegeTier = threat.family == ThreatFamily::Titan ? 5 : 4;
            out.canAttackPopulation = true; out.canSeizeObjective = true; out.canDisableDefense = true;
            if (threat.riftHorror) {
                const auto& r = *threat.riftHorror;
                out.canBurrow = r.locomotion == RiftLocomotion::Burrow;
                out.canPhase = r.locomotion == RiftLocomotion::PhaseStep;
                out.canFly = r.locomotion == RiftLocomotion::Fly;
                out.canBreach = out.canBurrow || out.canPhase || r.primaryAttack == RiftAttackVerb::Breach ||
                                r.secondaryAttack == RiftAttackVerb::Breach || r.scale != RiftScale::LargePredator;
                out.canInfiltrate = out.canBurrow || out.canPhase || r.locomotion == RiftLocomotion::WallCrawl ||
                                    r.locomotion == RiftLocomotion::Fly;
                out.canDrainPower = r.primaryAttack == RiftAttackVerb::DrainPower || r.secondaryAttack == RiftAttackVerb::DrainPower;
                out.canCorruptMachines = r.primaryAttack == RiftAttackVerb::CorruptMachines ||
                                         r.secondaryAttack == RiftAttackVerb::CorruptMachines;
                out.canDisableUtility = out.canDrainPower || out.canCorruptMachines || out.canBreach;
                out.structuralPowerScale = std::clamp(r.breachPower / 12.0f, 0.5f, 3.0f);
            } else {
                out.canBreach = threat.family == ThreatFamily::Titan;
                out.canDisableUtility = out.canBreach;
            }
            break;
        case ThreatFamily::InfiltratorSyndrome:
            out.siegeTier = 1; out.canInfiltrate = true; out.canDisableUtility = true; out.canAttackPopulation = true;
            out.canSeizeObjective = true; break;
        case ThreatFamily::MachineCorruption:
            out.siegeTier = 2; out.canDisableUtility = true; out.canDisableDefense = true;
            out.canCorruptMachines = true; out.structuralPowerScale = 0.4f; break;
        case ThreatFamily::EnvironmentalCatastrophe:
            out.siegeTier = 0; out.canDisableUtility = true; out.canAttackPopulation = true; out.structuralPowerScale = 0.0f; break;
        case ThreatFamily::CivilUnrest:
            out.siegeTier = 1; out.canInfiltrate = true; out.canAttackPopulation = true; out.canSeizeObjective = true; break;
    }
    return out;
}

bool threatCanPursueObjective(const ThreatCapabilityProfile& profile, ThreatObjectiveKind objective) {
    switch (objective) {
        case ThreatObjectiveKind::Approach: return true;
        case ThreatObjectiveKind::Breach: return profile.canBreach;
        case ThreatObjectiveKind::Infiltrate: return profile.canInfiltrate;
        case ThreatObjectiveKind::DisableUtility: return profile.canDisableUtility;
        case ThreatObjectiveKind::DisableDefense: return profile.canDisableDefense;
        case ThreatObjectiveKind::AttackPopulation: return profile.canAttackPopulation;
        case ThreatObjectiveKind::SeizeObjective: return profile.canSeizeObjective;
    }
    return false;
}

RiftHorrorDescriptor RiftHorrorGenerator::generate(std::uint64_t worldSeed,
                                                   std::uint64_t originSiteId,
                                                   std::uint64_t generationSerial,
                                                   std::uint32_t generatorVersion) {
    RiftHorrorDescriptor out{};
    out.generatorVersion = generatorVersion;
    out.generatorFingerprint = kRiftHorrorGeneratorFingerprint;
    out.originSiteId = originSiteId;
    out.generationSerial = generationSerial;

    const auto root = generatorRoot(worldSeed, originSiteId, generationSerial, generatorVersion);
    out.stableId = mix64(root ^ kRiftStableLabel);
    if (out.stableId == 0) out.stableId = 1;
    out.bodyPlan = static_cast<RiftBodyPlan>(axisIndex(root, kBodyLabel, kBodyCount));
    out.tissue = static_cast<RiftTissue>(axisIndex(root, kTissueLabel, kTissueCount));
    const float scaleRoll = axis01(root, kScaleLabel);
    out.scale = scaleRoll < 0.55f ? RiftScale::LargePredator : (scaleRoll < 0.90f ? RiftScale::Siege : RiftScale::Titan);
    out.locomotion = static_cast<RiftLocomotion>(axisIndex(root, kLocomotionLabel, kLocomotionCount));
    out.defense = static_cast<RiftDefense>(axisIndex(root, kDefenseLabel, kDefenseCount));
    out.primaryAttack = static_cast<RiftAttackVerb>(axisIndex(root, kAttackOneLabel, kAttackCount));
    auto second = axisIndex(root, kAttackTwoLabel, kAttackCount);
    if (second == static_cast<std::size_t>(out.primaryAttack)) second = (second + 1) % kAttackCount;
    out.secondaryAttack = static_cast<RiftAttackVerb>(second);
    out.emission = static_cast<RiftEmission>(axisIndex(root, kEmissionLabel, kEmissionCount));
    out.vulnerability = static_cast<RiftVulnerability>(axisIndex(root, kVulnerabilityLabel, kVulnerabilityCount));

    const float variation = 0.85f + 0.30f * axis01(root, 0x5354415456415231ULL);
    switch (out.scale) {
        case RiftScale::LargePredator:
            out.collisionRadiusMeters = 1.2f + 1.2f * axis01(root, 0x5241444955530001ULL);
            out.combatPower = 12.0f * variation;
            out.breachPower = 3.5f * variation;
            out.strategicPower = 8.0f * variation;
            break;
        case RiftScale::Siege:
            out.collisionRadiusMeters = 3.0f + 3.5f * axis01(root, 0x5241444955530002ULL);
            out.combatPower = 32.0f * variation;
            out.breachPower = 14.0f * variation;
            out.strategicPower = 28.0f * variation;
            break;
        case RiftScale::Titan:
            out.collisionRadiusMeters = 7.0f + 9.0f * axis01(root, 0x5241444955530003ULL);
            out.combatPower = 72.0f * variation;
            out.breachPower = 36.0f * variation;
            out.strategicPower = 70.0f * variation;
            break;
    }

    if (out.primaryAttack == RiftAttackVerb::Breach || out.secondaryAttack == RiftAttackVerb::Breach)
        out.breachPower *= 1.35f;
    if (out.locomotion == RiftLocomotion::Burrow || out.locomotion == RiftLocomotion::PhaseStep)
        out.breachPower *= 1.15f;

    out.name = makeRiftName(root, out.tissue, out.primaryAttack);
    out.signatureBehavior = signatureFor(out.primaryAttack, out.emission);
    out.counterplayHint = counterplayFor(out.vulnerability);
    return out;
}

ThreatUpdateResult ThreatLifecycleSystem::createGenericThreat(ThreatRecord& outRecord,
                                                              std::uint64_t stableId,
                                                              ThreatFamily family,
                                                              std::uint64_t originSiteId,
                                                              std::uint64_t targetSiteId,
                                                              float strategicPower,
                                                              float warningSeconds,
                                                              std::uint64_t campaignTick,
                                                              bool namedHistoricalFigure) {
    outRecord = {};
    outRecord.stableId = stableId == 0 ? 1 : stableId;
    outRecord.family = family;
    outRecord.lifecycle = ThreatLifecycle::Signaled;
    outRecord.representation = ThreatRepresentation::StrategicRemote;
    outRecord.siteId = targetSiteId;
    outRecord.originSiteId = originSiteId;
    outRecord.warningSecondsRemaining = std::max(0.0f, warningSeconds);
    outRecord.healthFraction = 1.0f;
    outRecord.strategicPower = std::max(0.0f, strategicPower);
    outRecord.namedHistoricalFigure = namedHistoricalFigure;
    outRecord.nextHistorySerial = 1;

    ThreatUpdateResult result{};
    result.history.push_back(makeHistory(outRecord, ThreatHistoryEventKind::Appearance, campaignTick,
                                         std::string(threatFamilyName(family)) + " entered the site's threat history."));
    const auto policy = threatSignalPolicy(family);
    result.history.push_back(makeHistory(outRecord, ThreatHistoryEventKind::Warning, campaignTick,
                                         std::string(threatFamilyName(family)) + " warning: " + policy.signal));
    result.commands.push_back({ThreatCommandKind::SetMilitaryAlert, outRecord.stableId, 0, std::nullopt,
                               RiftEmission::AcidMist, 1.0f, 0.0f});
    return result;
}

ThreatUpdateResult ThreatLifecycleSystem::createRiftThreat(ThreatRecord& outRecord,
                                                           const RiftHorrorDescriptor& descriptor,
                                                           std::uint64_t targetSiteId,
                                                           float warningSeconds,
                                                           std::uint64_t campaignTick) {
    outRecord = {};
    outRecord.stableId = descriptor.stableId;
    outRecord.family = descriptor.scale == RiftScale::Titan ? ThreatFamily::Titan : ThreatFamily::RiftHorror;
    outRecord.lifecycle = ThreatLifecycle::Signaled;
    outRecord.representation = ThreatRepresentation::StrategicRemote;
    outRecord.siteId = targetSiteId;
    outRecord.originSiteId = descriptor.originSiteId;
    outRecord.warningSecondsRemaining = std::max(0.0f, warningSeconds);
    outRecord.healthFraction = 1.0f;
    outRecord.strategicPower = descriptor.strategicPower;
    outRecord.namedHistoricalFigure = true;
    outRecord.nextHistorySerial = 1;
    outRecord.riftHorror = descriptor;

    ThreatUpdateResult result{};
    result.history.push_back(makeHistory(outRecord, ThreatHistoryEventKind::Appearance, campaignTick,
                                         descriptor.name + " entered the site's threat history."));
    std::ostringstream warning;
    warning << descriptor.name << " was detected approaching site " << targetSiteId
            << "; signature: " << descriptor.signatureBehavior;
    result.history.push_back(makeHistory(outRecord, ThreatHistoryEventKind::Warning, campaignTick, warning.str()));
    result.commands.push_back({ThreatCommandKind::SetMilitaryAlert, outRecord.stableId, 0, std::nullopt,
                               descriptor.emission, 1.0f, 0.0f});
    return result;
}

ThreatWarning ThreatLifecycleSystem::warningFor(const ThreatRecord& record) {
    ThreatWarning warning{};
    warning.threatStableId = record.stableId;
    warning.secondsUntilImpact = std::max(0.0f, record.warningSecondsRemaining);
    if (record.riftHorror) {
        warning.headline = std::string(threatFamilyName(record.family)) + ": " + record.riftHorror->name;
        warning.signal = record.riftHorror->signatureBehavior;
        warning.counterplay = record.riftHorror->counterplayHint;
    } else {
        const auto policy = threatSignalPolicy(record.family);
        warning.headline = threatFamilyName(record.family);
        warning.signal = policy.signal;
        warning.counterplay = policy.counterplay;
    }
    return warning;
}

ThreatUpdateResult ThreatLifecycleSystem::advanceApproach(ThreatRecord& record,
                                                          float dt,
                                                          bool siteDetailed,
                                                          const std::optional<SurfaceCellAddress>& spawnCell,
                                                          std::uint64_t campaignTick) {
    ThreatUpdateResult result{};
    if (record.lifecycle != ThreatLifecycle::Signaled && record.lifecycle != ThreatLifecycle::Approaching)
        return result;

    dt = std::max(0.0f, dt);
    record.lifecycle = ThreatLifecycle::Approaching;
    record.warningSecondsRemaining = std::max(0.0f, record.warningSecondsRemaining - dt);
    if (record.warningSecondsRemaining > 0.0f) return result;

    record.lifecycle = ThreatLifecycle::Engaged;
    result.impactNow = true;
    result.history.push_back(makeHistory(record, ThreatHistoryEventKind::Battle, campaignTick,
                                         std::string(threatFamilyName(record.family)) + " reached the defended site."));
    result.commands.push_back({ThreatCommandKind::SetMilitaryAlert, record.stableId, 0, std::nullopt,
                               record.riftHorror ? record.riftHorror->emission : RiftEmission::AcidMist,
                               1.0f, 0.0f});
    if (siteDetailed && spawnCell) {
        record.representation = ThreatRepresentation::ActiveEncounter;
        result.commands.push_back({ThreatCommandKind::SpawnOrPromoteCombatActor, record.stableId, 0, spawnCell,
                                   record.riftHorror ? record.riftHorror->emission : RiftEmission::AcidMist,
                                   record.riftHorror ? record.riftHorror->combatPower : record.strategicPower,
                                   record.riftHorror ? record.riftHorror->collisionRadiusMeters : 1.0f});
    } else {
        record.representation = ThreatRepresentation::StrategicRemote;
    }
    return result;
}

ThreatUpdateResult ThreatLifecycleSystem::promoteToActive(ThreatRecord& record,
                                                          const SurfaceCellAddress& spawnCell,
                                                          std::uint64_t campaignTick) {
    ThreatUpdateResult result{};
    if (record.lifecycle != ThreatLifecycle::Engaged && record.lifecycle != ThreatLifecycle::Retreating &&
        record.lifecycle != ThreatLifecycle::Migrating)
        return result;
    if (record.representation == ThreatRepresentation::ActiveEncounter) return result;

    record.representation = ThreatRepresentation::ActiveEncounter;
    result.commands.push_back({ThreatCommandKind::SpawnOrPromoteCombatActor, record.stableId, record.targetStableId, spawnCell,
                               record.riftHorror ? record.riftHorror->emission : RiftEmission::AcidMist,
                               record.riftHorror ? record.riftHorror->combatPower * record.healthFraction : record.strategicPower,
                               record.riftHorror ? record.riftHorror->collisionRadiusMeters : 1.0f});
    // Promotion is simulation LOD, not fiction. Do not manufacture a second
    // appearance event; the immutable StableId/history chain remains the same.
    (void)campaignTick;
    return result;
}

void ThreatLifecycleSystem::demoteToStrategic(ThreatRecord& record) {
    record.representation = ThreatRepresentation::StrategicRemote;
}

ThreatUpdateResult ThreatLifecycleSystem::resolve(ThreatRecord& record,
                                                  ThreatResolution resolution,
                                                  std::uint64_t campaignTick,
                                                  std::uint64_t relatedStableId) {
    ThreatUpdateResult result{};
    ThreatHistoryEventKind eventKind = ThreatHistoryEventKind::Defeat;
    std::string verb;
    switch (resolution) {
        case ThreatResolution::Retreat:
            record.lifecycle = ThreatLifecycle::Retreating;
            eventKind = ThreatHistoryEventKind::Retreat;
            verb = "retreated from";
            result.commands.push_back({ThreatCommandKind::RetreatActor, record.stableId, 0, std::nullopt,
                                       record.riftHorror ? record.riftHorror->emission : RiftEmission::AcidMist, 1.0f, 0.0f});
            break;
        case ThreatResolution::Migrate:
            record.lifecycle = ThreatLifecycle::Migrating;
            eventKind = ThreatHistoryEventKind::Migration;
            verb = "migrated away from";
            break;
        case ThreatResolution::Contain:
            record.lifecycle = ThreatLifecycle::Contained;
            eventKind = ThreatHistoryEventKind::Containment;
            verb = "was contained at";
            break;
        case ThreatResolution::Defeat:
            record.lifecycle = ThreatLifecycle::Defeated;
            eventKind = ThreatHistoryEventKind::Defeat;
            verb = "was defeated at";
            break;
        case ThreatResolution::Kill:
            record.lifecycle = ThreatLifecycle::Dead;
            record.healthFraction = 0.0f;
            eventKind = ThreatHistoryEventKind::Death;
            verb = "was killed at";
            break;
        case ThreatResolution::Resolve:
            record.lifecycle = ThreatLifecycle::Resolved;
            eventKind = ThreatHistoryEventKind::Defeat;
            verb = "was resolved at";
            break;
    }
    result.history.push_back(makeHistory(record, eventKind, campaignTick,
                                         std::string(threatFamilyName(record.family)) + " " + verb + " site " +
                                         std::to_string(record.siteId) + ".", relatedStableId));
    return result;
}

RemoteThreatOutcome ThreatLifecycleSystem::resolveRemoteInterval(ThreatRecord& record,
                                                                 const RemoteThreatContext& context,
                                                                 float elapsedHours,
                                                                 std::uint64_t campaignTick,
                                                                 std::vector<ThreatHistoryEvent>* history) {
    RemoteThreatOutcome outcome{};
    if (record.lifecycle != ThreatLifecycle::Engaged || elapsedHours <= 0.0f || !std::isfinite(elapsedHours))
        return outcome;

    record.representation = ThreatRepresentation::StrategicRemote;
    const float threatPressure = std::max(0.0f, record.strategicPower) * std::clamp(record.healthFraction, 0.0f, 1.0f);
    const float defense = std::max(0.0f, context.siteDefensePower);
    const float exposure = std::clamp(context.populationExposure, 0.0f, 2.0f);
    const float containment = std::max(0.0f, context.containmentPower);

    if (containment > threatPressure * 1.35f && containment > 0.0f) {
        outcome.contained = true;
        record.lifecycle = ThreatLifecycle::Contained;
        if (history) history->push_back(makeHistory(record, ThreatHistoryEventKind::Containment, campaignTick,
                                                    "Remote defenders contained the named threat."));
        return outcome;
    }

    if (defense >= threatPressure) {
        const float ratio = defense <= 0.0f ? 0.0f : (defense - threatPressure) / std::max(1.0f, defense);
        outcome.threatHealthLost = std::clamp(elapsedHours * (0.06f + 0.18f * ratio), 0.0f, record.healthFraction);
        record.healthFraction = std::max(0.0f, record.healthFraction - outcome.threatHealthLost);
        if (record.healthFraction <= 0.001f) {
            record.healthFraction = 0.0f;
            record.lifecycle = ThreatLifecycle::Defeated;
            outcome.defeated = true;
            if (history) history->push_back(makeHistory(record, ThreatHistoryEventKind::Defeat, campaignTick,
                                                        "Remote defenders defeated the named threat."));
        } else if (defense > threatPressure * 1.60f && elapsedHours >= 0.5f) {
            record.lifecycle = ThreatLifecycle::Retreating;
            outcome.forcedRetreat = true;
            if (history) history->push_back(makeHistory(record, ThreatHistoryEventKind::Retreat, campaignTick,
                                                        "Remote defenders forced the named threat to retreat."));
        }
    } else {
        const float imbalance = (threatPressure - defense) / std::max(1.0f, threatPressure);
        outcome.siteDamage = elapsedHours * (0.5f + 2.5f * imbalance) * exposure;
        if (history && outcome.siteDamage >= 1.0f)
            history->push_back(makeHistory(record, ThreatHistoryEventKind::Rampage, campaignTick,
                                           "The named threat damaged the remote site during strategic simulation."));
    }
    return outcome;
}

ThreatSiegePlanner::ThreatSiegePlanner(ThreatPlannerTuning tuning) : tuning_(tuning) {
    tuning_.maxNodesExamined = std::max<std::size_t>(1, tuning_.maxNodesExamined);
    tuning_.maxPlans = std::max<std::size_t>(1, std::min(tuning_.maxPlans, tuning_.maxNodesExamined));
}

std::vector<ThreatSiegePlan> ThreatSiegePlanner::plan(const ThreatRecord& threat,
                                                      const ThreatDefenseGraph& graph) {
    telemetry_ = {};
    std::vector<const ThreatDefenseNode*> ordered;
    ordered.reserve(graph.nodes.size());
    for (const auto& node : graph.nodes) ordered.push_back(&node);
    std::sort(ordered.begin(), ordered.end(), [](const auto* a, const auto* b) {
        return std::tie(a->stableNodeId, a->targetStableId, a->address.face, a->address.u, a->address.v, a->address.radial) <
               std::tie(b->stableNodeId, b->targetStableId, b->address.face, b->address.u, b->address.v, b->address.radial);
    });

    const std::size_t limit = std::min(tuning_.maxNodesExamined, ordered.size());
    std::vector<ThreatSiegePlan> plans;
    plans.reserve(limit);
    const auto capabilities = threatCapabilityProfile(threat);
    const float breachPower = threat.riftHorror ? threat.riftHorror->breachPower :
                              std::max(0.0f, threat.strategicPower * capabilities.structuralPowerScale);
    const bool phases = capabilities.canPhase || capabilities.canBurrow;

    for (std::size_t i = 0; i < limit; ++i) {
        const auto& node = *ordered[i];
        ++telemetry_.nodesExamined;
        const auto objective = objectiveForNode(node.kind);
        if (!threatCanPursueObjective(capabilities, objective)) {
            ++telemetry_.nodesRejectedByCapability;
            continue;
        }
        float score = baseNodeValue(node.kind);
        score += 2.0f * std::max(0.0f, node.exposure);
        score += 1.5f * std::max(0.0f, node.utilityValue);
        score -= 1.5f * std::max(0.0f, node.defenseCoverage);
        score -= 0.75f * std::max(0.0f, node.accessCost);
        if (objective == ThreatObjectiveKind::Breach)
            score += phases ? 5.0f : std::min(8.0f, breachPower / std::max(0.5f, node.hardness));
        if (objective == ThreatObjectiveKind::DisableUtility && hasAttack(threat, RiftAttackVerb::DrainPower)) score += 6.0f;
        if (objective == ThreatObjectiveKind::DisableUtility && hasAttack(threat, RiftAttackVerb::CorruptMachines)) score += 5.0f;
        if (objective == ThreatObjectiveKind::Infiltrate && phases) score += 5.0f;
        score += 0.25f * static_cast<float>(capabilities.siegeTier);

        plans.push_back({objective, node.stableNodeId, node.targetStableId, node.address, score,
                         planReason(objective, node)});
    }

    std::sort(plans.begin(), plans.end(), [](const auto& a, const auto& b) {
        if (a.score != b.score) return a.score > b.score;
        return std::tie(a.targetNodeId, a.targetStableId, a.targetCell.face, a.targetCell.u, a.targetCell.v, a.targetCell.radial) <
               std::tie(b.targetNodeId, b.targetStableId, b.targetCell.face, b.targetCell.u, b.targetCell.v, b.targetCell.radial);
    });
    if (plans.size() > tuning_.maxPlans) plans.resize(tuning_.maxPlans);
    telemetry_.plansGenerated = plans.size();
    return plans;
}

std::vector<ThreatCommand> ThreatSiegePlanner::commandsForPlan(const ThreatRecord& threat,
                                                               const ThreatSiegePlan& plan) const {
    std::vector<ThreatCommand> out;
    const auto capabilities = threatCapabilityProfile(threat);
    if (!threatCanPursueObjective(capabilities, plan.objective)) return out;

    out.push_back({ThreatCommandKind::RequestPath, threat.stableId, plan.targetStableId, plan.targetCell,
                   threat.riftHorror ? threat.riftHorror->emission : RiftEmission::AcidMist, 0.0f, 0.0f});

    const float combatPower = threat.riftHorror ? threat.riftHorror->combatPower * threat.healthFraction : threat.strategicPower;
    const float breachPower = threat.riftHorror ? threat.riftHorror->breachPower * threat.healthFraction :
                              threat.strategicPower * capabilities.structuralPowerScale;
    switch (plan.objective) {
        case ThreatObjectiveKind::Breach:
            out.push_back({ThreatCommandKind::DamageStructure, threat.stableId, plan.targetStableId, plan.targetCell,
                           threat.riftHorror ? threat.riftHorror->emission : RiftEmission::AcidMist, breachPower, 1.5f});
            break;
        case ThreatObjectiveKind::DisableUtility:
            if (hasAttack(threat, RiftAttackVerb::DrainPower))
                out.push_back({ThreatCommandKind::DrainPower, threat.stableId, plan.targetStableId, plan.targetCell,
                               threat.riftHorror->emission, combatPower * 0.5f, 8.0f});
            else if (hasAttack(threat, RiftAttackVerb::CorruptMachines))
                out.push_back({ThreatCommandKind::CorruptMachine, threat.stableId, plan.targetStableId, plan.targetCell,
                               threat.riftHorror->emission, combatPower * 0.4f, 6.0f});
            else
                out.push_back({ThreatCommandKind::DamageStructure, threat.stableId, plan.targetStableId, plan.targetCell,
                               threat.riftHorror ? threat.riftHorror->emission : RiftEmission::AcidMist, breachPower, 1.0f});
            break;
        case ThreatObjectiveKind::DisableDefense:
        case ThreatObjectiveKind::AttackPopulation:
        case ThreatObjectiveKind::SeizeObjective:
            out.push_back({ThreatCommandKind::DamageActor, threat.stableId, plan.targetStableId, plan.targetCell,
                           threat.riftHorror ? threat.riftHorror->emission : RiftEmission::AcidMist, combatPower, 1.0f});
            break;
        case ThreatObjectiveKind::Infiltrate:
            if (threat.riftHorror)
                out.push_back({ThreatCommandKind::EmitEnvironmentalHazard, threat.stableId, plan.targetStableId, plan.targetCell,
                               threat.riftHorror->emission, combatPower * 0.15f, 4.0f});
            break;
        case ThreatObjectiveKind::Approach:
            break;
    }
    return out;
}

ThreatRecord mirrorImperialThreat(const SurfaceSiegeState& state, std::uint64_t siteId) {
    ThreatRecord out{};
    out.stableId = state.actionId;
    out.family = ThreatFamily::ImperialEnforcement;
    out.siteId = siteId;
    out.originSiteId = siteId;
    out.warningSecondsRemaining = std::max(0.0f, state.phaseSecondsRemaining);
    out.healthFraction = 1.0f;
    out.strategicPower = static_cast<float>(std::max(1, state.totalWaves));
    out.namedHistoricalFigure = false;
    switch (state.phase) {
        case RegisterActionPhase::Idle: out.lifecycle = ThreatLifecycle::Dormant; break;
        case RegisterActionPhase::Announced: out.lifecycle = ThreatLifecycle::Signaled; break;
        case RegisterActionPhase::InterWave: out.lifecycle = ThreatLifecycle::Approaching; break;
        case RegisterActionPhase::WaveActive: out.lifecycle = ThreatLifecycle::Engaged; break;
        case RegisterActionPhase::Cleared: out.lifecycle = ThreatLifecycle::Defeated; break;
        case RegisterActionPhase::Failed: out.lifecycle = ThreatLifecycle::Resolved; break;
    }
    out.representation = state.phase == RegisterActionPhase::WaveActive ?
        ThreatRepresentation::ActiveEncounter : ThreatRepresentation::StrategicRemote;
    return out;
}

ThreatWarning imperialThreatWarning(const SurfaceSiegeState& state) {
    ThreatWarning warning{};
    warning.threatStableId = state.actionId;
    warning.secondsUntilImpact = state.phase == RegisterActionPhase::Announced ?
        std::max(0.0f, state.phaseSecondsRemaining) : 0.0f;
    warning.headline = state.type == ImperialEnforcementType::RegisterAction ?
        "Imperial Register Action" : "Imperial Patrol";
    warning.signal = state.phase == RegisterActionPhase::Announced ?
        "Imperial enforcement is announced before impact; wave count is fixed by the existing Register Action director." :
        "Imperial enforcement is active or already resolved.";
    warning.counterplay = state.claimRequired ?
        "Protect the Registry Beacon, keep defenses powered and supplied, and defeat tracked attackers." :
        "Avoid or defeat the patrol; this adapter does not change the existing enforcement rules.";
    return warning;
}

std::string serializeRiftHorrorDescriptor(const RiftHorrorDescriptor& descriptor) {
    std::ostringstream out;
    out << "ELYSIUM_RIFT_HORROR " << descriptor.generatorVersion << '\n';
    out << descriptor.generatorFingerprint << ' ' << descriptor.stableId << ' ' << descriptor.originSiteId << ' '
        << descriptor.generationSerial << ' ' << static_cast<int>(descriptor.bodyPlan) << ' '
        << static_cast<int>(descriptor.tissue) << ' ' << static_cast<int>(descriptor.scale) << ' '
        << static_cast<int>(descriptor.locomotion) << ' ' << static_cast<int>(descriptor.defense) << ' '
        << static_cast<int>(descriptor.primaryAttack) << ' ' << static_cast<int>(descriptor.secondaryAttack) << ' '
        << static_cast<int>(descriptor.emission) << ' ' << static_cast<int>(descriptor.vulnerability) << ' '
        << std::setprecision(9) << descriptor.collisionRadiusMeters << ' ' << descriptor.combatPower << ' '
        << descriptor.breachPower << ' ' << descriptor.strategicPower << '\n';
    out << std::quoted(descriptor.name) << '\n' << std::quoted(descriptor.signatureBehavior) << '\n'
        << std::quoted(descriptor.counterplayHint) << '\n';
    return out.str();
}

std::optional<RiftHorrorDescriptor> deserializeRiftHorrorDescriptor(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) -> std::optional<RiftHorrorDescriptor> {
        if (error) *error = message;
        return std::nullopt;
    };
    std::istringstream in{std::string(text)};
    std::string magic;
    std::uint32_t version{};
    if (!(in >> magic >> version) || magic != "ELYSIUM_RIFT_HORROR" || version == 0)
        return fail("unsupported Rift Horror descriptor header");

    RiftHorrorDescriptor out{};
    out.generatorVersion = version;
    int body{}, tissue{}, scale{}, locomotion{}, defense{}, primary{}, secondary{}, emission{}, vulnerability{};
    if (!(in >> out.generatorFingerprint >> out.stableId >> out.originSiteId >> out.generationSerial >>
          body >> tissue >> scale >> locomotion >> defense >> primary >> secondary >> emission >> vulnerability >>
          out.collisionRadiusMeters >> out.combatPower >> out.breachPower >> out.strategicPower))
        return fail("malformed Rift Horror descriptor");
    if (!enumInRange(body, static_cast<int>(kBodyCount)) || !enumInRange(tissue, static_cast<int>(kTissueCount)) ||
        !enumInRange(scale, 3) || !enumInRange(locomotion, static_cast<int>(kLocomotionCount)) ||
        !enumInRange(defense, static_cast<int>(kDefenseCount)) || !enumInRange(primary, static_cast<int>(kAttackCount)) ||
        !enumInRange(secondary, static_cast<int>(kAttackCount)) || !enumInRange(emission, static_cast<int>(kEmissionCount)) ||
        !enumInRange(vulnerability, static_cast<int>(kVulnerabilityCount)) || out.stableId == 0 ||
        !finiteNonNegative(out.collisionRadiusMeters) || !finiteNonNegative(out.combatPower) ||
        !finiteNonNegative(out.breachPower) || !finiteNonNegative(out.strategicPower))
        return fail("Rift Horror descriptor values out of range");
    out.bodyPlan = static_cast<RiftBodyPlan>(body);
    out.tissue = static_cast<RiftTissue>(tissue);
    out.scale = static_cast<RiftScale>(scale);
    out.locomotion = static_cast<RiftLocomotion>(locomotion);
    out.defense = static_cast<RiftDefense>(defense);
    out.primaryAttack = static_cast<RiftAttackVerb>(primary);
    out.secondaryAttack = static_cast<RiftAttackVerb>(secondary);
    out.emission = static_cast<RiftEmission>(emission);
    out.vulnerability = static_cast<RiftVulnerability>(vulnerability);
    if (!(in >> std::quoted(out.name) >> std::quoted(out.signatureBehavior) >> std::quoted(out.counterplayHint)))
        return fail("malformed Rift Horror descriptor text fields");
    if (out.name.empty() || out.signatureBehavior.empty() || out.counterplayHint.empty())
        return fail("Rift Horror descriptor lacks readable signature/counterplay");
    if (error) error->clear();
    return out;
}

std::string serializeThreatRecord(const ThreatRecord& record) {
    std::ostringstream out;
    out << "ELYSIUM_THREAT_RECORD 1\n";
    out << record.stableId << ' ' << static_cast<int>(record.family) << ' ' << static_cast<int>(record.lifecycle) << ' '
        << static_cast<int>(record.representation) << ' ' << record.siteId << ' ' << record.originSiteId << ' '
        << record.targetStableId << ' ' << std::setprecision(9) << record.warningSecondsRemaining << ' '
        << record.healthFraction << ' ' << record.strategicPower << ' ' << (record.namedHistoricalFigure ? 1 : 0) << ' '
        << record.nextHistorySerial << ' ' << (record.riftHorror ? 1 : 0) << '\n';
    if (record.riftHorror) {
        const auto nested = serializeRiftHorrorDescriptor(*record.riftHorror);
        out << nested.size() << '\n' << nested;
    }
    return out.str();
}

std::optional<ThreatRecord> deserializeThreatRecord(std::string_view text, std::string* error) {
    auto fail = [&](const char* message) -> std::optional<ThreatRecord> {
        if (error) *error = message;
        return std::nullopt;
    };
    std::istringstream in{std::string(text)};
    std::string magic;
    int schema{};
    if (!(in >> magic >> schema) || magic != "ELYSIUM_THREAT_RECORD" || schema != 1)
        return fail("unsupported threat record header");

    ThreatRecord out{};
    int family{}, lifecycle{}, representation{}, named{}, hasRift{};
    if (!(in >> out.stableId >> family >> lifecycle >> representation >> out.siteId >> out.originSiteId >>
          out.targetStableId >> out.warningSecondsRemaining >> out.healthFraction >> out.strategicPower >> named >>
          out.nextHistorySerial >> hasRift))
        return fail("malformed threat record");
    if (out.stableId == 0 || !enumInRange(family, static_cast<int>(kThreatFamilyCount)) ||
        !enumInRange(lifecycle, static_cast<int>(kThreatLifecycleCount)) || !enumInRange(representation, 2) ||
        !finiteNonNegative(out.warningSecondsRemaining) || !finiteUnit(out.healthFraction) ||
        !finiteNonNegative(out.strategicPower) || out.nextHistorySerial == 0 || (named != 0 && named != 1) ||
        (hasRift != 0 && hasRift != 1))
        return fail("threat record values out of range");
    out.family = static_cast<ThreatFamily>(family);
    out.lifecycle = static_cast<ThreatLifecycle>(lifecycle);
    out.representation = static_cast<ThreatRepresentation>(representation);
    out.namedHistoricalFigure = named != 0;

    if (hasRift) {
        std::size_t size{};
        if (!(in >> size) || size == 0 || size > 64 * 1024) return fail("invalid nested Rift Horror descriptor size");
        in.get(); // consume newline
        std::string nested(size, '\0');
        in.read(nested.data(), static_cast<std::streamsize>(size));
        if (static_cast<std::size_t>(in.gcount()) != size) return fail("truncated nested Rift Horror descriptor");
        std::string nestedError;
        auto parsed = deserializeRiftHorrorDescriptor(nested, &nestedError);
        if (!parsed) {
            if (error) *error = "invalid nested Rift Horror descriptor: " + nestedError;
            return std::nullopt;
        }
        if (parsed->stableId != out.stableId) return fail("threat record stable ID disagrees with nested Rift Horror");
        out.riftHorror = std::move(*parsed);
    }
    if (error) error->clear();
    return out;
}

} // namespace elysium
