// Intended function: imported exploration implementation for Exploration; preserves the agent-authored subsystem contract for later integration/debugging.
#include "exploration/Exploration.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <limits>
#include <queue>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr std::uint64_t AnnotationLabel = 0x414E4E4F54415445ULL;
constexpr std::uint64_t FeatureLabel = 0x4D41504645415455ULL;
constexpr std::size_t DiagnosticLimit = 64;

bool finite(float v) { return std::isfinite(v); }
bool finite(double v) { return std::isfinite(v); }

void normalizeStrings(std::vector<std::string>& values) {
    values.erase(std::remove_if(values.begin(), values.end(), [](const auto& value) { return value.empty(); }), values.end());
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

bool hasAllTags(const ResearchEvidence& evidence, const std::vector<std::string>& required) {
    for (const auto& requiredTag : required) {
        if (!std::binary_search(evidence.tags.begin(), evidence.tags.end(), requiredTag)) return false;
    }
    return true;
}

bool hazardExcluded(const std::set<std::string>& known, const std::vector<std::string>& excluded) {
    for (const auto& hazard : excluded) if (known.contains(hazard)) return true;
    return false;
}

double distanceLy(const SystemChart& a, const SystemChart& b) {
    const double dx = a.xLy - b.xLy;
    const double dy = a.yLy - b.yLy;
    const double dz = a.zLy - b.zLy;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

int fuelForLeg(double distance, double efficiency) {
    if (!(distance >= 0.0) || !(efficiency > 0.0)) return std::numeric_limits<int>::max();
    return std::max(1, static_cast<int>(std::ceil((distance / 100.0) / efficiency - 1e-12)));
}

std::string routeFailureName(RouteFailure failure) {
    switch (failure) {
        case RouteFailure::None: return "none";
        case RouteFailure::UnknownOrigin: return "unknown origin";
        case RouteFailure::UnknownDestination: return "unknown destination";
        case RouteFailure::PositionUnknown: return "charted position unknown";
        case RouteFailure::InvalidShipProfile: return "invalid ship profile";
        case RouteFailure::Unreachable: return "destination unreachable using charted systems";
        case RouteFailure::InsufficientFuel: return "insufficient fuel";
    }
    return "unknown route failure";
}

} // namespace

const char* scanTargetKindName(ScanTargetKind kind) {
    switch (kind) {
        case ScanTargetKind::System: return "system";
        case ScanTargetKind::Planet: return "planet";
        case ScanTargetKind::Flora: return "flora";
        case ScanTargetKind::Fauna: return "fauna";
        case ScanTargetKind::MineralVein: return "mineral vein";
        case ScanTargetKind::Structure: return "structure";
        case ScanTargetKind::Anomaly: return "anomaly";
    }
    return "unknown";
}

const char* scanFailureName(ScanFailure failure) {
    switch (failure) {
        case ScanFailure::None: return "none";
        case ScanFailure::AlreadyKnown: return "already known";
        case ScanFailure::InvalidTarget: return "invalid target";
        case ScanFailure::InsufficientGrade: return "scanner grade too low";
        case ScanFailure::OutOfRange: return "target out of range";
        case ScanFailure::InsufficientCharge: return "insufficient scanner charge";
        case ScanFailure::OrbitalContextRequired: return "orbital context required";
        case ScanFailure::SpecialModuleRequired: return "special scanner module required";
    }
    return "unknown";
}

const char* knowledgeObjectTypeName(KnowledgeObjectType type) {
    switch (type) {
        case KnowledgeObjectType::Fact: return "fact";
        case KnowledgeObjectType::Technique: return "technique";
        case KnowledgeObjectType::Theory: return "theory";
        case KnowledgeObjectType::MapChart: return "map/chart";
        case KnowledgeObjectType::HistoricalAccount: return "historical account";
        case KnowledgeObjectType::CulturalWork: return "cultural work";
        case KnowledgeObjectType::ForbiddenProtocol: return "forbidden protocol";
    }
    return "unknown";
}

const char* researchFieldName(ResearchField field) {
    switch (field) {
        case ResearchField::Geology: return "geology";
        case ResearchField::Xenobiology: return "xenobiology";
        case ResearchField::Environmental: return "environmental";
        case ResearchField::ImperialSystems: return "imperial systems";
        case ResearchField::RiftScience: return "rift science";
        case ResearchField::Engineering: return "engineering";
    }
    return "unknown";
}

const char* mapLayerName(MapLayer layer) {
    switch (layer) {
        case MapLayer::Biome: return "biome";
        case MapLayer::Elevation: return "elevation";
        case MapLayer::Hazard: return "hazard";
        case MapLayer::KnownResources: return "known resources";
        case MapLayer::ClaimsPower: return "claims/power";
        case MapLayer::CavesPois: return "caves/POIs";
        case MapLayer::Annotations: return "annotations";
    }
    return "unknown";
}

ExplorationService::ExplorationService(std::uint64_t worldSeed) : worldSeed_(worldSeed) {
    for (int i = static_cast<int>(MapLayer::Biome); i <= static_cast<int>(MapLayer::Annotations); ++i)
        layerVisibility_[static_cast<MapLayer>(i)] = true;
}

bool ExplorationService::registerResearchUnlock(ResearchUnlockDefinition definition, std::string* error) {
    if (definition.id.empty()) { if (error) *error = "research unlock id is empty"; return false; }
    if (definition.pointsRequired < 0) { if (error) *error = "research unlock points must be non-negative"; return false; }
    if (!finite(definition.minimumEvidenceConfidence) || definition.minimumEvidenceConfidence < 0.0f || definition.minimumEvidenceConfidence > 1.0f) {
        if (error) *error = "research unlock confidence threshold must be in [0,1]";
        return false;
    }
    normalizeStrings(definition.requiredEvidenceTags);
    if (std::any_of(researchUnlockDefinitions_.begin(), researchUnlockDefinitions_.end(), [&](const auto& existing) { return existing.id == definition.id; })) {
        if (error) *error = "duplicate research unlock id";
        return false;
    }
    researchUnlockDefinitions_.push_back(std::move(definition));
    std::sort(researchUnlockDefinitions_.begin(), researchUnlockDefinitions_.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    evaluateResearchUnlocks();
    return true;
}

void ExplorationService::applyReward(const ExplorationReward& reward, std::uint32_t systemId, IExplorationMutationSink& sink) const {
    if (reward.credits != 0) sink.addCredits(reward.credits);
    if (reward.favor != 0.0f) sink.addFavor(reward.favor);
    if (reward.suspicion != 0.0f) sink.addSystemSuspicion(systemId, reward.suspicion);
}

ScanOutcome ExplorationService::scan(const ScanTargetDescriptor& target, ScannerProfile& scanner, IExplorationMutationSink& sink) {
    ++telemetry_.scanAttempts;
    ScanOutcome out{};
    auto fail = [&](ScanFailure reason, std::string read) {
        out.failure = reason;
        out.diagnostic = scanFailureName(reason);
        if (reason == ScanFailure::InsufficientGrade) ++telemetry_.rejectedGrade;
        if (reason == ScanFailure::OutOfRange) ++telemetry_.rejectedRange;
        if (reason == ScanFailure::InsufficientCharge) ++telemetry_.rejectedCharge;
        pushDiagnostic({"scan", target.stableTargetId, std::move(read), out.diagnostic, "no persistent change"});
        return out;
    };

    if (target.stableTargetId == 0 || target.chargeCost < 0 || target.requiredGrade < 0 || target.distanceMeters < 0.0f || !finite(target.distanceMeters))
        return fail(ScanFailure::InvalidTarget, "target descriptor validation");
    if (findDiscovery(target.stableTargetId)) {
        ++telemetry_.duplicateScans;
        return fail(ScanFailure::AlreadyKnown, "stable target already catalogued");
    }
    if (scanner.grade < target.requiredGrade) return fail(ScanFailure::InsufficientGrade, "scanner grade vs target gate");
    if (!finite(scanner.rangeMeters) || scanner.rangeMeters < 0.0f || target.distanceMeters > scanner.rangeMeters)
        return fail(ScanFailure::OutOfRange, "scanner range vs observed distance");
    if (target.requiresOrbitalContext && !scanner.orbitalContext) return fail(ScanFailure::OrbitalContextRequired, "target requires orbital survey context");
    if (target.requiresSpecialModule && !scanner.specialAnomalyModule) return fail(ScanFailure::SpecialModuleRequired, "target requires special anomaly module");
    if (scanner.charges < target.chargeCost) return fail(ScanFailure::InsufficientCharge, "scanner charge vs scan cost");

    scanner.charges -= target.chargeCost;
    out.chargesConsumed = target.chargeCost;

    DiscoveryRecord record{};
    record.stableTargetId = target.stableTargetId;
    record.kind = target.kind;
    record.systemId = target.systemId;
    record.planetIndex = target.planetIndex;
    record.canonicalName = target.canonicalName;
    record.catalogueTags = target.catalogueTags;
    normalizeStrings(record.catalogueTags);
    record.discoveryRewardGranted = true;
    record.filingRewards = target.filingRewards;
    discoveries_.emplace(record.stableTargetId, record);

    applyReward(target.discoveryReward, target.systemId, sink);
    out.reward = target.discoveryReward;
    out.success = true;
    out.newlyDiscovered = true;
    ++telemetry_.successfulScans;
    ++telemetry_.newDiscoveries;

    if (target.kind == ScanTargetKind::System) {
        auto& chart = upsertSystemChart(target.systemId);
        chart.charted = true;
    } else {
        auto& chart = upsertSystemChart(target.systemId);
        chart.charted = true;
        auto& planet = upsertPlanetChart(target.systemId, target.planetIndex);
        planet.discovered = true;
        if (target.kind == ScanTargetKind::Planet) planet.surveyed = true;
    }

    if (target.researchField && target.researchPoints > 0) {
        ResearchEvidence evidence{};
        evidence.stableSourceId = target.stableTargetId;
        evidence.field = *target.researchField;
        evidence.points = target.researchPoints;
        evidence.tags = target.evidenceTags;
        evidence.description = std::string("scan:") + scanTargetKindName(target.kind);
        evidence.objectType = (target.kind == ScanTargetKind::System || target.kind == ScanTargetKind::Planet)
            ? KnowledgeObjectType::MapChart
            : (target.kind == ScanTargetKind::Anomaly ? KnowledgeObjectType::Theory : KnowledgeObjectType::Fact);
        evidence.confidence = 1.0f;
        evidence.provenanceStableId = target.stableTargetId;
        std::string error;
        if (!addResearchEvidence(std::move(evidence), &out.newlyUnlockedResearch, &error)) {
            out.diagnostic = "discovered; research evidence rejected: " + error;
        }
    }
    if (out.diagnostic.empty()) out.diagnostic = "discovery committed";
    pushDiagnostic({"scan", target.stableTargetId,
                    "grade/range/charge/context + stable target identity",
                    "scan accepted",
                    "discovery, catalogue state, reward and research evidence committed"});
    return out;
}

bool ExplorationService::nameDiscovery(std::uint64_t stableTargetId, std::string name, std::string* error) {
    auto* record = findDiscovery(stableTargetId);
    if (!record) { if (error) *error = "discovery not found"; return false; }
    if (name.empty()) { if (error) *error = "name is empty"; return false; }
    if (name.size() > 96) { if (error) *error = "name exceeds 96 bytes"; return false; }
    record->playerName = std::move(name);
    pushDiagnostic({"name discovery", stableTargetId, "catalogued discovery", "name accepted", "player-facing discovery name changed"});
    return true;
}

FilingOutcome ExplorationService::fileDiscovery(std::uint64_t stableTargetId, FilingChoice choice, IExplorationMutationSink& sink) {
    FilingOutcome out{};
    out.choice = choice;
    auto* record = findDiscovery(stableTargetId);
    if (!record) {
        out.diagnostic = "discovery not found";
        pushDiagnostic({"file discovery", stableTargetId, "discovery lookup", out.diagnostic, "no persistent change"});
        return out;
    }
    if (choice == FilingChoice::Unfiled) {
        out.diagnostic = "Unfiled is not a filing choice";
        pushDiagnostic({"file discovery", stableTargetId, "filing choice", out.diagnostic, "no persistent change"});
        return out;
    }
    if (record->filing != FilingChoice::Unfiled) {
        out.diagnostic = "discovery already filed";
        pushDiagnostic({"file discovery", stableTargetId, "persistent filing state", out.diagnostic, "no duplicate reward"});
        return out;
    }

    ExplorationReward reward{};
    if (choice == FilingChoice::Empire) reward = record->filingRewards.empire;
    else if (choice == FilingChoice::Unsworn) reward = record->filingRewards.unsworn;

    record->filing = choice;
    record->chartsPropagated = choice == FilingChoice::Unsworn;
    applyReward(reward, record->systemId, sink);
    if (record->chartsPropagated) sink.propagateCharts(record->stableTargetId);
    out.success = true;
    out.reward = reward;
    out.chartsPropagated = record->chartsPropagated;
    out.diagnostic = choice == FilingChoice::Private ? "kept private" : "filing committed";
    ++telemetry_.filings;
    pushDiagnostic({"file discovery", stableTargetId,
                    "discovery + filing channel",
                    out.diagnostic,
                    record->chartsPropagated ? "reward applied; chart propagation requested" : "filing state/reward applied"});
    return out;
}

bool ExplorationService::addResearchEvidence(ResearchEvidence evidence, std::vector<std::string>* newlyUnlocked, std::string* error) {
    if (evidence.stableSourceId == 0) { if (error) *error = "research source id is zero"; return false; }
    if (evidence.points <= 0) { if (error) *error = "research evidence points must be positive"; return false; }
    if (!finite(evidence.confidence) || evidence.confidence < 0.0f || evidence.confidence > 1.0f) { if (error) *error = "research evidence confidence must be in [0,1]"; return false; }
    normalizeStrings(evidence.tags);
    if (researchEvidence_.contains(evidence.stableSourceId)) {
        if (error) *error = "research source already recorded";
        return false;
    }
    const auto sourceId = evidence.stableSourceId;
    const auto field = evidence.field;
    researchEvidence_.emplace(sourceId, std::move(evidence));
    ++telemetry_.researchEvidenceAccepted;
    auto unlocked = evaluateResearchUnlocks();
    telemetry_.researchUnlocksGranted += unlocked.size();
    if (newlyUnlocked) *newlyUnlocked = unlocked;
    const auto& storedEvidence = researchEvidence_.at(sourceId);
    pushDiagnostic({"research evidence", sourceId,
                    std::string("field=") + researchFieldName(field) + ", type=" + knowledgeObjectTypeName(storedEvidence.objectType) + ", confidence=" + std::to_string(storedEvidence.confidence),
                    "evidence accepted",
                    unlocked.empty() ? "knowledge increased; no new unlock" : "knowledge increased; relevant unlock granted"});
    return true;
}

int ExplorationService::researchPoints(ResearchField field) const {
    int total = 0;
    for (const auto& [id, evidence] : researchEvidence_) {
        (void)id;
        if (evidence.field == field) total += evidence.points;
    }
    return total;
}

bool ExplorationService::researchUnlocked(const std::string& id) const { return unlockedResearch_.contains(id); }

std::vector<std::string> ExplorationService::evaluateResearchUnlocks() {
    std::vector<std::string> newly;
    for (const auto& def : researchUnlockDefinitions_) {
        if (unlockedResearch_.contains(def.id)) continue;
        int qualifiedPoints = 0;
        for (const auto& [sourceId, evidence] : researchEvidence_) {
            (void)sourceId;
            if (evidence.field == def.field && evidence.confidence + 1e-6f >= def.minimumEvidenceConfidence) qualifiedPoints += evidence.points;
        }
        if (qualifiedPoints < def.pointsRequired) continue;
        bool relevant = def.requiredEvidenceTags.empty();
        if (!relevant) {
            for (const auto& [sourceId, evidence] : researchEvidence_) {
                (void)sourceId;
                if (evidence.field == def.field && evidence.confidence + 1e-6f >= def.minimumEvidenceConfidence && hasAllTags(evidence, def.requiredEvidenceTags)) { relevant = true; break; }
            }
        }
        if (!relevant) continue;
        unlockedResearch_.insert(def.id);
        newly.push_back(def.id);
    }
    return newly;
}

DiscoveryRecord* ExplorationService::findDiscovery(std::uint64_t stableTargetId) {
    const auto it = discoveries_.find(stableTargetId);
    return it == discoveries_.end() ? nullptr : &it->second;
}

const DiscoveryRecord* ExplorationService::findDiscovery(std::uint64_t stableTargetId) const {
    const auto it = discoveries_.find(stableTargetId);
    return it == discoveries_.end() ? nullptr : &it->second;
}

SystemChart& ExplorationService::upsertSystemChart(std::uint32_t systemId) {
    auto [it, inserted] = systemCharts_.try_emplace(systemId);
    if (inserted) it->second.systemId = systemId;
    return it->second;
}

PlanetChart& ExplorationService::upsertPlanetChart(std::uint32_t systemId, std::uint16_t planetIndex) {
    auto& system = upsertSystemChart(systemId);
    auto [it, inserted] = system.planets.try_emplace(planetIndex);
    if (inserted) it->second.planetIndex = planetIndex;
    return it->second;
}

const SystemChart* ExplorationService::findSystemChart(std::uint32_t systemId) const {
    const auto it = systemCharts_.find(systemId);
    return it == systemCharts_.end() ? nullptr : &it->second;
}

std::uint64_t ExplorationService::allocateStableId(std::uint64_t label, std::uint64_t& counter, const std::set<std::uint64_t>& occupied) const {
    for (;;) {
        const std::uint64_t candidate = mix64(worldSeed_ ^ label ^ mix64(counter++));
        if (candidate != 0 && !occupied.contains(candidate)) return candidate;
    }
}

std::uint64_t ExplorationService::addMapFeature(MapFeature feature) {
    if (!finite(feature.value) || !finite(feature.confidence)) return 0;
    if (feature.stableFeatureId == 0) {
        std::set<std::uint64_t> occupied;
        for (const auto& [id, existing] : mapFeatures_) { (void)existing; occupied.insert(id); }
        feature.stableFeatureId = allocateStableId(FeatureLabel, featureCounter_, occupied);
    } else if (mapFeatures_.contains(feature.stableFeatureId)) return 0;
    feature.confidence = std::clamp(feature.confidence, 0.0f, 1.0f);
    const auto id = feature.stableFeatureId;
    mapFeatures_.emplace(id, std::move(feature));
    return id;
}

bool ExplorationService::removeMapFeature(std::uint64_t stableFeatureId) { return mapFeatures_.erase(stableFeatureId) != 0; }

std::vector<MapFeature> ExplorationService::mapFeatures(MapLayer layer, std::uint32_t systemId, std::optional<std::uint16_t> planetIndex) const {
    std::vector<MapFeature> out;
    if (!layerVisible(layer)) return out;
    for (const auto& [id, feature] : mapFeatures_) {
        (void)id;
        if (feature.layer != layer || feature.systemId != systemId) continue;
        if (planetIndex && feature.planetIndex != *planetIndex) continue;
        const auto* system = findSystemChart(feature.systemId);
        if (!system || !system->charted) continue;
        if (feature.layer != MapLayer::Annotations) {
            const auto pit = system->planets.find(feature.planetIndex);
            if (pit == system->planets.end() || !pit->second.discovered) continue;
        }
        out.push_back(feature);
    }
    return out;
}

void ExplorationService::setLayerVisible(MapLayer layer, bool visible) { layerVisibility_[layer] = visible; }

bool ExplorationService::layerVisible(MapLayer layer) const {
    const auto it = layerVisibility_.find(layer);
    return it == layerVisibility_.end() ? true : it->second;
}

std::uint64_t ExplorationService::addAnnotation(PlayerAnnotation annotation) {
    if (annotation.text.empty() || annotation.text.size() > 256) return 0;
    const auto* system = findSystemChart(annotation.systemId);
    if (!system || !system->charted) return 0;
    if (annotation.stableAnnotationId == 0) {
        std::set<std::uint64_t> occupied;
        for (const auto& [id, existing] : annotations_) { (void)existing; occupied.insert(id); }
        annotation.stableAnnotationId = allocateStableId(AnnotationLabel, annotationCounter_, occupied);
    } else if (annotations_.contains(annotation.stableAnnotationId)) return 0;
    const auto id = annotation.stableAnnotationId;
    annotations_.emplace(id, std::move(annotation));
    return id;
}

bool ExplorationService::removeAnnotation(std::uint64_t stableAnnotationId) { return annotations_.erase(stableAnnotationId) != 0; }

std::vector<DiscoveryRecord> ExplorationService::searchDiscoveries(const DiscoveryQuery& query) {
    ++telemetry_.catalogueQueries;
    std::vector<DiscoveryRecord> out;
    std::string text = query.text;
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    for (const auto& [id, discovery] : discoveries_) {
        (void)id;
        if (query.kind && discovery.kind != *query.kind) continue;
        if (query.systemId && discovery.systemId != *query.systemId) continue;
        if (query.planetIndex && discovery.planetIndex != *query.planetIndex) continue;
        if (query.filing && discovery.filing != *query.filing) continue;
        if (query.requiredCatalogueTag && !std::binary_search(discovery.catalogueTags.begin(), discovery.catalogueTags.end(), *query.requiredCatalogueTag)) continue;
        if (!text.empty()) {
            std::string haystack = discovery.playerName.empty() ? discovery.canonicalName : discovery.playerName + " " + discovery.canonicalName;
            std::transform(haystack.begin(), haystack.end(), haystack.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            bool matched = haystack.find(text) != std::string::npos;
            if (!matched) {
                for (const auto& tag : discovery.catalogueTags) {
                    std::string lowered = tag;
                    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                    if (lowered.find(text) != std::string::npos) { matched = true; break; }
                }
            }
            if (!matched) continue;
        }
        out.push_back(discovery);
    }
    std::sort(out.begin(), out.end(), [](const DiscoveryRecord& a, const DiscoveryRecord& b) {
        if (a.systemId != b.systemId) return a.systemId < b.systemId;
        if (a.planetIndex != b.planetIndex) return a.planetIndex < b.planetIndex;
        return a.stableTargetId < b.stableTargetId;
    });
    pushDiagnostic({"catalogue search", 0,
                    "persistent discovered records only",
                    out.empty() ? "no discovered match" : "discovered matches returned",
                    "query telemetry only; no undiscovered target state consulted"});
    return out;
}

ResearchProgressReport ExplorationService::researchProgress(const std::string& unlockId) const {
    ResearchProgressReport report{};
    const auto it = std::find_if(researchUnlockDefinitions_.begin(), researchUnlockDefinitions_.end(),
                                 [&](const auto& def) { return def.id == unlockId; });
    if (it == researchUnlockDefinitions_.end()) {
        report.diagnostic = "research unlock definition not found";
        return report;
    }
    report.definitionFound = true;
    report.unlocked = unlockedResearch_.contains(unlockId);
    report.field = it->field;
    report.pointsRequired = it->pointsRequired;
    report.minimumEvidenceConfidence = it->minimumEvidenceConfidence;
    std::set<std::string> observedTags;
    for (const auto& [sourceId, evidence] : researchEvidence_) {
        if (evidence.field != it->field || evidence.confidence + 1e-6f < it->minimumEvidenceConfidence) continue;
        report.qualifiedPoints += evidence.points;
        ++report.qualifyingEvidenceCount;
        report.qualifyingSourceIds.push_back(sourceId);
        observedTags.insert(evidence.tags.begin(), evidence.tags.end());
    }
    for (const auto& tag : it->requiredEvidenceTags) if (!observedTags.contains(tag)) report.missingEvidenceTags.push_back(tag);
    if (report.unlocked) report.diagnostic = "unlocked";
    else if (report.qualifiedPoints < report.pointsRequired) report.diagnostic = "insufficient qualifying research points";
    else if (!report.missingEvidenceTags.empty()) report.diagnostic = "required evidence tags not yet observed at qualifying confidence";
    else report.diagnostic = "requirements satisfied; unlock evaluation pending";
    return report;
}

std::vector<NeedMatch> ExplorationService::need(const NeedQuery& query) {
    ++telemetry_.needQueries;
    std::vector<NeedMatch> out;
    for (const auto& [systemId, system] : systemCharts_) {
        if (!system.charted) continue;
        if (query.requireKnownAccessibleStation && (!system.stationAccessKnown || !system.stationAccessible)) continue;
        if (query.maximumKnownSuspicion && (!system.suspicionKnown || system.suspicion > *query.maximumKnownSuspicion)) continue;
        if (hazardExcluded(system.knownHazards, query.excludedHazards)) continue;

        const bool planetScoped = query.planetClass.has_value();
        if (!planetScoped) {
            float confidence = 0.0f;
            std::vector<std::string> evidence;
            if (query.resourceId) {
                const auto rit = system.resourceConfidence.find(*query.resourceId);
                if (rit == system.resourceConfidence.end() || rit->second < query.minimumResourceConfidence) continue;
                confidence = rit->second;
                evidence.push_back("known system resource: " + *query.resourceId);
            }
            if (query.requireKnownAccessibleStation) evidence.push_back("known accessible station");
            if (query.maximumKnownSuspicion) evidence.push_back("known Suspicion within limit");
            out.push_back({systemId, std::nullopt, confidence, std::move(evidence)});
            continue;
        }

        for (const auto& [planetIndex, planet] : system.planets) {
            if (!planet.discovered || !planet.planetClassKnown || planet.planetClass != *query.planetClass) continue;
            if (hazardExcluded(planet.knownHazards, query.excludedHazards)) continue;
            float confidence = 0.0f;
            std::vector<std::string> evidence{"known planet class: " + planet.planetClass};
            if (query.resourceId) {
                const auto rit = planet.resourceConfidence.find(*query.resourceId);
                if (rit == planet.resourceConfidence.end() || rit->second < query.minimumResourceConfidence) continue;
                confidence = rit->second;
                evidence.push_back("surveyed resource: " + *query.resourceId);
            }
            out.push_back({systemId, planetIndex, confidence, std::move(evidence)});
        }
    }
    std::sort(out.begin(), out.end(), [](const NeedMatch& a, const NeedMatch& b) {
        if (a.resourceConfidence != b.resourceConfidence) return a.resourceConfidence > b.resourceConfidence;
        if (a.systemId != b.systemId) return a.systemId < b.systemId;
        return a.planetIndex.value_or(0) < b.planetIndex.value_or(0);
    });
    pushDiagnostic({"Need X", 0,
                    "charted systems/planets and explicit known facts only",
                    out.empty() ? "no known match" : "known matches ranked",
                    "query telemetry only; no hidden world state revealed"});
    return out;
}

RouteEstimate ExplorationService::estimateRoute(const RouteRequest& request) {
    ++telemetry_.routeQueries;
    RouteEstimate out{};
    auto fail = [&](RouteFailure failure) {
        ++telemetry_.routeFailures;
        out.failure = failure;
        pushDiagnostic({"route estimate", request.destinationSystemId,
                        "charted positions + ship profile + known hazards/stations/Suspicion",
                        routeFailureName(failure),
                        "no route state mutated"});
        return out;
    };

    const auto* origin = findSystemChart(request.originSystemId);
    if (!origin || !origin->charted) return fail(RouteFailure::UnknownOrigin);
    const auto* destination = findSystemChart(request.destinationSystemId);
    if (!destination || !destination->charted) return fail(RouteFailure::UnknownDestination);
    if (!origin->positionKnown || !destination->positionKnown) return fail(RouteFailure::PositionUnknown);
    if (!(request.jumpRangeLy > 0.0) || !(request.warpEfficiency > 0.0) || request.availableFuelCells < 0 ||
        !finite(request.jumpRangeLy) || !finite(request.warpEfficiency)) return fail(RouteFailure::InvalidShipProfile);

    std::vector<std::uint32_t> ids;
    ids.reserve(systemCharts_.size());
    for (const auto& [id, system] : systemCharts_) if (system.charted && system.positionKnown) ids.push_back(id);
    if (std::find(ids.begin(), ids.end(), request.originSystemId) == ids.end() ||
        std::find(ids.begin(), ids.end(), request.destinationSystemId) == ids.end()) return fail(RouteFailure::PositionUnknown);

    struct Cost { int fuel{std::numeric_limits<int>::max()}; int hops{std::numeric_limits<int>::max()}; double distance{std::numeric_limits<double>::infinity()}; };
    auto better = [](const Cost& a, const Cost& b) {
        if (a.fuel != b.fuel) return a.fuel < b.fuel;
        if (a.hops != b.hops) return a.hops < b.hops;
        return a.distance + 1e-9 < b.distance;
    };
    struct Node { Cost cost; std::uint32_t id{}; };
    auto cmp = [&](const Node& a, const Node& b) {
        if (a.cost.fuel != b.cost.fuel) return a.cost.fuel > b.cost.fuel;
        if (a.cost.hops != b.cost.hops) return a.cost.hops > b.cost.hops;
        if (std::abs(a.cost.distance - b.cost.distance) > 1e-9) return a.cost.distance > b.cost.distance;
        return a.id > b.id;
    };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);
    std::map<std::uint32_t, Cost> best;
    std::map<std::uint32_t, std::uint32_t> previous;
    best[request.originSystemId] = {0, 0, 0.0};
    open.push({best[request.originSystemId], request.originSystemId});

    while (!open.empty()) {
        const Node node = open.top(); open.pop();
        const auto bit = best.find(node.id);
        if (bit == best.end()) continue;
        const Cost current = bit->second;
        if (node.cost.fuel != current.fuel || node.cost.hops != current.hops || std::abs(node.cost.distance - current.distance) > 1e-9) continue;
        if (node.id == request.destinationSystemId) break;
        const auto& from = systemCharts_.at(node.id);
        for (const auto candidateId : ids) {
            if (candidateId == node.id) continue;
            const auto& to = systemCharts_.at(candidateId);
            const double legDistance = distanceLy(from, to);
            if (legDistance > request.jumpRangeLy + 1e-9) continue;
            const int legFuel = fuelForLeg(legDistance, request.warpEfficiency);
            Cost next{current.fuel + legFuel, current.hops + 1, current.distance + legDistance};
            const auto existing = best.find(candidateId);
            if (existing == best.end() || better(next, existing->second) ||
                (!better(existing->second, next) && previous[candidateId] > node.id)) {
                best[candidateId] = next;
                previous[candidateId] = node.id;
                open.push({next, candidateId});
            }
        }
    }

    const auto dit = best.find(request.destinationSystemId);
    if (dit == best.end()) return fail(RouteFailure::Unreachable);
    if (dit->second.fuel > request.availableFuelCells) {
        out.fuelCells = dit->second.fuel;
        out.distanceLy = dit->second.distance;
        return fail(RouteFailure::InsufficientFuel);
    }

    std::vector<std::uint32_t> route;
    for (std::uint32_t at = request.destinationSystemId;;) {
        route.push_back(at);
        if (at == request.originSystemId) break;
        const auto pit = previous.find(at);
        if (pit == previous.end()) return fail(RouteFailure::Unreachable);
        at = pit->second;
    }
    std::reverse(route.begin(), route.end());
    out.found = true;
    out.failure = RouteFailure::None;
    out.systems = std::move(route);
    out.distanceLy = dit->second.distance;
    out.fuelCells = dit->second.fuel;
    for (std::size_t i = 1; i < out.systems.size(); ++i) {
        const auto& system = systemCharts_.at(out.systems[i]);
        if (system.stationAccessKnown) { if (system.stationAccessible) ++out.knownStationStops; }
        else ++out.unknownStationStops;
        if (system.suspicionKnown) out.maximumKnownSuspicion = std::max(out.maximumKnownSuspicion, system.suspicion);
        else ++out.unknownSuspicionStops;
        out.knownHazardTags += static_cast<int>(system.knownHazards.size());
    }
    pushDiagnostic({"route estimate", request.destinationSystemId,
                    "charted positions + ship profile + known hazards/stations/Suspicion",
                    "deterministic known-space route found",
                    "estimate returned; unknown facts remain explicitly unknown"});
    return out;
}

std::optional<ExplorationDiagnostic> ExplorationService::lastDiagnostic() const {
    if (diagnostics_.empty()) return std::nullopt;
    return diagnostics_.back();
}

void ExplorationService::pushDiagnostic(ExplorationDiagnostic diagnostic) {
    if (diagnostics_.size() == DiagnosticLimit) diagnostics_.erase(diagnostics_.begin());
    diagnostics_.push_back(std::move(diagnostic));
}

std::string ExplorationService::serialize() const {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "ELYSIUM_EXPLORATION 1\n";
    out << "meta " << worldSeed_ << ' ' << annotationCounter_ << ' ' << featureCounter_ << '\n';
    out << "layers " << layerVisibility_.size();
    for (const auto& [layer, visible] : layerVisibility_) out << ' ' << static_cast<int>(layer) << ' ' << (visible ? 1 : 0);
    out << '\n';

    out << "discoveries " << discoveries_.size() << '\n';
    for (const auto& [id, d] : discoveries_) {
        out << "discovery " << id << ' ' << static_cast<int>(d.kind) << ' ' << d.systemId << ' ' << d.planetIndex << ' '
            << (d.discoveryRewardGranted ? 1 : 0) << ' '
            << d.filingRewards.empire.credits << ' ' << d.filingRewards.empire.favor << ' ' << d.filingRewards.empire.suspicion << ' '
            << d.filingRewards.unsworn.credits << ' ' << d.filingRewards.unsworn.favor << ' ' << d.filingRewards.unsworn.suspicion << ' '
            << static_cast<int>(d.filing) << ' ' << (d.chartsPropagated ? 1 : 0) << ' '
            << std::quoted(d.canonicalName) << ' ' << std::quoted(d.playerName) << ' ' << d.catalogueTags.size();
        for (const auto& tag : d.catalogueTags) out << ' ' << std::quoted(tag);
        out << '\n';
    }

    out << "evidence " << researchEvidence_.size() << '\n';
    for (const auto& [id, e] : researchEvidence_) {
        out << "research " << id << ' ' << static_cast<int>(e.field) << ' ' << e.points << ' ' << static_cast<int>(e.objectType) << ' ' << e.confidence << ' ' << e.provenanceStableId << ' ' << std::quoted(e.description) << ' ' << e.tags.size();
        for (const auto& tag : e.tags) out << ' ' << std::quoted(tag);
        out << '\n';
    }
    out << "unlocked " << unlockedResearch_.size();
    for (const auto& id : unlockedResearch_) out << ' ' << std::quoted(id);
    out << '\n';

    out << "systems " << systemCharts_.size() << '\n';
    for (const auto& [id, s] : systemCharts_) {
        out << "system " << id << ' ' << (s.charted ? 1 : 0) << ' ' << (s.positionKnown ? 1 : 0) << ' '
            << s.xLy << ' ' << s.yLy << ' ' << s.zLy << ' ' << (s.stationAccessKnown ? 1 : 0) << ' ' << (s.stationAccessible ? 1 : 0) << ' '
            << (s.suspicionKnown ? 1 : 0) << ' ' << s.suspicion << ' ' << s.resourceConfidence.size();
        for (const auto& [resource, confidence] : s.resourceConfidence) out << ' ' << std::quoted(resource) << ' ' << confidence;
        out << ' ' << s.knownHazards.size();
        for (const auto& hazard : s.knownHazards) out << ' ' << std::quoted(hazard);
        out << ' ' << s.planets.size() << '\n';
        for (const auto& [planetIndex, p] : s.planets) {
            out << "planet " << id << ' ' << planetIndex << ' ' << (p.discovered ? 1 : 0) << ' ' << (p.surveyed ? 1 : 0) << ' '
                << (p.planetClassKnown ? 1 : 0) << ' ' << std::quoted(p.planetClass) << ' ' << p.resourceConfidence.size();
            for (const auto& [resource, confidence] : p.resourceConfidence) out << ' ' << std::quoted(resource) << ' ' << confidence;
            out << ' ' << p.knownHazards.size();
            for (const auto& hazard : p.knownHazards) out << ' ' << std::quoted(hazard);
            out << ' ' << p.knownPoiFamilies.size();
            for (const auto& poi : p.knownPoiFamilies) out << ' ' << std::quoted(poi);
            out << '\n';
        }
    }

    out << "features " << mapFeatures_.size() << '\n';
    for (const auto& [id, f] : mapFeatures_) {
        out << "feature " << id << ' ' << static_cast<int>(f.layer) << ' ' << f.systemId << ' ' << f.planetIndex << ' ' << (f.hasSurfaceAddress ? 1 : 0) << ' '
            << static_cast<int>(f.surfaceAddress.face) << ' ' << f.surfaceAddress.u << ' ' << f.surfaceAddress.v << ' ' << f.surfaceAddress.radial << ' '
            << f.value << ' ' << f.confidence << ' ' << std::quoted(f.label) << '\n';
    }
    out << "annotations " << annotations_.size() << '\n';
    for (const auto& [id, a] : annotations_) {
        out << "annotation " << id << ' ' << a.systemId << ' ' << a.planetIndex << ' ' << (a.hasSurfaceAddress ? 1 : 0) << ' '
            << static_cast<int>(a.surfaceAddress.face) << ' ' << a.surfaceAddress.u << ' ' << a.surfaceAddress.v << ' ' << a.surfaceAddress.radial << ' '
            << std::quoted(a.text) << '\n';
    }
    return out.str();
}

bool ExplorationService::restore(const std::string& text, std::string* error) {
    std::istringstream in(text);
    std::string magic;
    int version{};
    if (!(in >> magic >> version) || magic != "ELYSIUM_EXPLORATION" || version != 1) {
        if (error) *error = "unsupported exploration save header";
        return false;
    }

    ExplorationService temp;
    std::string tag;
    if (!(in >> tag) || tag != "meta" || !(in >> temp.worldSeed_ >> temp.annotationCounter_ >> temp.featureCounter_)) {
        if (error) *error = "invalid exploration meta record";
        return false;
    }
    std::size_t count{};
    if (!(in >> tag >> count) || tag != "layers" || count > 32) { if (error) *error = "invalid layer table"; return false; }
    temp.layerVisibility_.clear();
    for (std::size_t i = 0; i < count; ++i) {
        int layer{}, visible{};
        if (!(in >> layer >> visible) || layer < static_cast<int>(MapLayer::Biome) || layer > static_cast<int>(MapLayer::Annotations) || (visible != 0 && visible != 1)) {
            if (error) *error = "invalid layer entry";
            return false;
        }
        temp.layerVisibility_[static_cast<MapLayer>(layer)] = visible != 0;
    }

    if (!(in >> tag >> count) || tag != "discoveries" || count > 1000000) { if (error) *error = "invalid discovery count"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        DiscoveryRecord d{};
        int kind{}, reward{}, filing{}, propagated{};
        std::size_t tags{};
        if (!(in >> tag >> d.stableTargetId >> kind >> d.systemId >> d.planetIndex >> reward
              >> d.filingRewards.empire.credits >> d.filingRewards.empire.favor >> d.filingRewards.empire.suspicion
              >> d.filingRewards.unsworn.credits >> d.filingRewards.unsworn.favor >> d.filingRewards.unsworn.suspicion
              >> filing >> propagated >> std::quoted(d.canonicalName) >> std::quoted(d.playerName) >> tags) ||
            tag != "discovery" || d.stableTargetId == 0 || kind < 0 || kind > static_cast<int>(ScanTargetKind::Anomaly) ||
            filing < 0 || filing > static_cast<int>(FilingChoice::Unsworn) || tags > 256 ||
            !finite(d.filingRewards.empire.favor) || !finite(d.filingRewards.empire.suspicion) ||
            !finite(d.filingRewards.unsworn.favor) || !finite(d.filingRewards.unsworn.suspicion)) {
            if (error) *error = "invalid discovery entry";
            return false;
        }
        d.kind = static_cast<ScanTargetKind>(kind);
        d.discoveryRewardGranted = reward != 0;
        d.filing = static_cast<FilingChoice>(filing);
        d.chartsPropagated = propagated != 0;
        for (std::size_t n = 0; n < tags; ++n) { std::string value; if (!(in >> std::quoted(value))) { if (error) *error = "invalid discovery tag"; return false; } d.catalogueTags.push_back(std::move(value)); }
        normalizeStrings(d.catalogueTags);
        if (!temp.discoveries_.emplace(d.stableTargetId, std::move(d)).second) { if (error) *error = "duplicate discovery id"; return false; }
    }

    if (!(in >> tag >> count) || tag != "evidence" || count > 1000000) { if (error) *error = "invalid evidence count"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        ResearchEvidence e{};
        int field{}, objectType{};
        std::size_t tags{};
        if (!(in >> tag >> e.stableSourceId >> field >> e.points >> objectType >> e.confidence >> e.provenanceStableId >> std::quoted(e.description) >> tags) || tag != "research" || e.stableSourceId == 0 ||
            field < 0 || field > static_cast<int>(ResearchField::Engineering) || objectType < 0 || objectType > static_cast<int>(KnowledgeObjectType::ForbiddenProtocol) ||
            e.points <= 0 || tags > 256 || !finite(e.confidence) || e.confidence < 0.0f || e.confidence > 1.0f) {
            if (error) *error = "invalid research evidence entry";
            return false;
        }
        e.field = static_cast<ResearchField>(field);
        e.objectType = static_cast<KnowledgeObjectType>(objectType);
        for (std::size_t n = 0; n < tags; ++n) { std::string value; if (!(in >> std::quoted(value))) { if (error) *error = "invalid research evidence tag"; return false; } e.tags.push_back(std::move(value)); }
        normalizeStrings(e.tags);
        if (!temp.researchEvidence_.emplace(e.stableSourceId, std::move(e)).second) { if (error) *error = "duplicate research source id"; return false; }
    }
    if (!(in >> tag >> count) || tag != "unlocked" || count > 100000) { if (error) *error = "invalid unlocked research count"; return false; }
    for (std::size_t i = 0; i < count; ++i) { std::string id; if (!(in >> std::quoted(id)) || id.empty()) { if (error) *error = "invalid unlocked research id"; return false; } temp.unlockedResearch_.insert(std::move(id)); }

    if (!(in >> tag >> count) || tag != "systems" || count > 131072) { if (error) *error = "invalid system chart count"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        SystemChart s{};
        int charted{}, positionKnown{}, stationKnown{}, stationAccessible{}, suspicionKnown{};
        std::size_t resources{}, hazards{}, planets{};
        if (!(in >> tag >> s.systemId >> charted >> positionKnown >> s.xLy >> s.yLy >> s.zLy >> stationKnown >> stationAccessible >> suspicionKnown >> s.suspicion >> resources) ||
            tag != "system" || resources > 1024 || !finite(s.xLy) || !finite(s.yLy) || !finite(s.zLy) || !finite(s.suspicion)) {
            if (error) *error = "invalid system chart entry";
            return false;
        }
        s.charted = charted != 0; s.positionKnown = positionKnown != 0; s.stationAccessKnown = stationKnown != 0; s.stationAccessible = stationAccessible != 0; s.suspicionKnown = suspicionKnown != 0;
        for (std::size_t n = 0; n < resources; ++n) { std::string id; float confidence{}; if (!(in >> std::quoted(id) >> confidence) || !finite(confidence)) { if (error) *error = "invalid system resource knowledge"; return false; } s.resourceConfidence[id] = std::clamp(confidence, 0.0f, 1.0f); }
        if (!(in >> hazards) || hazards > 256) { if (error) *error = "invalid system hazards"; return false; }
        for (std::size_t n = 0; n < hazards; ++n) { std::string value; if (!(in >> std::quoted(value))) { if (error) *error = "invalid system hazard"; return false; } s.knownHazards.insert(std::move(value)); }
        if (!(in >> planets) || planets > 256) { if (error) *error = "invalid planet chart count"; return false; }
        if (!temp.systemCharts_.emplace(s.systemId, s).second) { if (error) *error = "duplicate system chart"; return false; }
        for (std::size_t p = 0; p < planets; ++p) {
            PlanetChart planet{};
            std::uint32_t owner{};
            int discovered{}, surveyed{}, classKnown{};
            std::size_t planetResources{}, planetHazards{}, pois{};
            if (!(in >> tag >> owner >> planet.planetIndex >> discovered >> surveyed >> classKnown >> std::quoted(planet.planetClass) >> planetResources) ||
                tag != "planet" || owner != s.systemId || planetResources > 1024) { if (error) *error = "invalid planet chart entry"; return false; }
            planet.discovered = discovered != 0; planet.surveyed = surveyed != 0; planet.planetClassKnown = classKnown != 0;
            for (std::size_t n = 0; n < planetResources; ++n) { std::string id; float confidence{}; if (!(in >> std::quoted(id) >> confidence) || !finite(confidence)) { if (error) *error = "invalid planet resource knowledge"; return false; } planet.resourceConfidence[id] = std::clamp(confidence, 0.0f, 1.0f); }
            if (!(in >> planetHazards) || planetHazards > 256) { if (error) *error = "invalid planet hazard count"; return false; }
            for (std::size_t n = 0; n < planetHazards; ++n) { std::string value; if (!(in >> std::quoted(value))) { if (error) *error = "invalid planet hazard"; return false; } planet.knownHazards.insert(std::move(value)); }
            if (!(in >> pois) || pois > 256) { if (error) *error = "invalid planet POI count"; return false; }
            for (std::size_t n = 0; n < pois; ++n) { std::string value; if (!(in >> std::quoted(value))) { if (error) *error = "invalid planet POI"; return false; } planet.knownPoiFamilies.insert(std::move(value)); }
            auto& stored = temp.systemCharts_.at(s.systemId).planets;
            if (!stored.emplace(planet.planetIndex, std::move(planet)).second) { if (error) *error = "duplicate planet chart"; return false; }
        }
    }

    if (!(in >> tag >> count) || tag != "features" || count > 1000000) { if (error) *error = "invalid map feature count"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        MapFeature f{};
        int layer{}, hasAddress{}, face{};
        if (!(in >> tag >> f.stableFeatureId >> layer >> f.systemId >> f.planetIndex >> hasAddress >> face >> f.surfaceAddress.u >> f.surfaceAddress.v >> f.surfaceAddress.radial >> f.value >> f.confidence >> std::quoted(f.label)) ||
            tag != "feature" || f.stableFeatureId == 0 || layer < 0 || layer > static_cast<int>(MapLayer::Annotations) || face < 0 || face > 5 || !finite(f.value) || !finite(f.confidence)) {
            if (error) *error = "invalid map feature";
            return false;
        }
        f.layer = static_cast<MapLayer>(layer); f.hasSurfaceAddress = hasAddress != 0; f.surfaceAddress.face = static_cast<CubeFace>(face); f.confidence = std::clamp(f.confidence, 0.0f, 1.0f);
        if (!temp.mapFeatures_.emplace(f.stableFeatureId, std::move(f)).second) { if (error) *error = "duplicate map feature id"; return false; }
    }

    if (!(in >> tag >> count) || tag != "annotations" || count > 1000000) { if (error) *error = "invalid annotation count"; return false; }
    for (std::size_t i = 0; i < count; ++i) {
        PlayerAnnotation a{};
        int hasAddress{}, face{};
        if (!(in >> tag >> a.stableAnnotationId >> a.systemId >> a.planetIndex >> hasAddress >> face >> a.surfaceAddress.u >> a.surfaceAddress.v >> a.surfaceAddress.radial >> std::quoted(a.text)) ||
            tag != "annotation" || a.stableAnnotationId == 0 || face < 0 || face > 5 || a.text.empty() || a.text.size() > 256) {
            if (error) *error = "invalid annotation";
            return false;
        }
        a.hasSurfaceAddress = hasAddress != 0; a.surfaceAddress.face = static_cast<CubeFace>(face);
        if (!temp.annotations_.emplace(a.stableAnnotationId, std::move(a)).second) { if (error) *error = "duplicate annotation id"; return false; }
    }

    in >> std::ws;
    if (!in.eof()) { if (error) *error = "trailing data in exploration save"; return false; }
    temp.researchUnlockDefinitions_ = researchUnlockDefinitions_;
    // Loaded unlock IDs are persistent truth; definitions may be from a newer
    // build and are intentionally not used to delete old unlocked capabilities.
    *this = std::move(temp);
    pushDiagnostic({"restore exploration", 0, "versioned stable-ID records", "restore accepted", "discovery/research/cartography state replaced atomically"});
    return true;
}

} // namespace elysium
