// Intended function: imported exploration implementation for Exploration; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace elysium {

enum class ScanTargetKind : std::uint8_t {
    System = 0,
    Planet = 1,
    Flora = 2,
    Fauna = 3,
    MineralVein = 4,
    Structure = 5,
    Anomaly = 6
};

enum class ScanFailure : std::uint8_t {
    None = 0,
    AlreadyKnown,
    InvalidTarget,
    InsufficientGrade,
    OutOfRange,
    InsufficientCharge,
    OrbitalContextRequired,
    SpecialModuleRequired
};

enum class FilingChoice : std::uint8_t {
    Unfiled = 0,
    Private = 1,
    Empire = 2,
    Unsworn = 3
};

enum class KnowledgeObjectType : std::uint8_t {
    Fact = 0,
    Technique = 1,
    Theory = 2,
    MapChart = 3,
    HistoricalAccount = 4,
    CulturalWork = 5,
    ForbiddenProtocol = 6
};

enum class ResearchField : std::uint8_t {
    Geology = 0,
    Xenobiology = 1,
    Environmental = 2,
    ImperialSystems = 3,
    RiftScience = 4,
    Engineering = 5
};

enum class ResearchUnlockKind : std::uint8_t {
    Blueprint = 0,
    ScannerInterpretation = 1,
    AutomationCapability = 2,
    Efficiency = 3,
    ExpeditionCapability = 4,
    InformationTool = 5
};

enum class MapLayer : std::uint8_t {
    Biome = 0,
    Elevation = 1,
    Hazard = 2,
    KnownResources = 3,
    ClaimsPower = 4,
    CavesPois = 5,
    Annotations = 6
};

const char* scanTargetKindName(ScanTargetKind kind);
const char* scanFailureName(ScanFailure failure);
const char* knowledgeObjectTypeName(KnowledgeObjectType type);
const char* researchFieldName(ResearchField field);
const char* mapLayerName(MapLayer layer);

struct ExplorationReward {
    int credits{};
    float favor{};
    float suspicion{};

    friend bool operator==(const ExplorationReward&, const ExplorationReward&) = default;
};

struct FilingRewardProfile {
    ExplorationReward empire{};
    ExplorationReward unsworn{};
};

struct ScannerProfile {
    int grade{};
    int charges{};
    float rangeMeters{};
    bool orbitalContext{};
    bool specialAnomalyModule{};
};

struct ScanTargetDescriptor {
    std::uint64_t stableTargetId{};
    ScanTargetKind kind{ScanTargetKind::System};
    std::uint32_t systemId{};
    std::uint16_t planetIndex{};
    std::string canonicalName;
    int requiredGrade{};
    int chargeCost{};
    float distanceMeters{};
    bool requiresOrbitalContext{};
    bool requiresSpecialModule{};
    ExplorationReward discoveryReward{};
    FilingRewardProfile filingRewards{};
    std::optional<ResearchField> researchField;
    int researchPoints{};
    std::vector<std::string> evidenceTags;
    std::vector<std::string> catalogueTags;
};

struct DiscoveryRecord {
    std::uint64_t stableTargetId{};
    ScanTargetKind kind{ScanTargetKind::System};
    std::uint32_t systemId{};
    std::uint16_t planetIndex{};
    std::string canonicalName;
    std::string playerName;
    std::vector<std::string> catalogueTags;
    bool discoveryRewardGranted{};
    FilingRewardProfile filingRewards{};
    FilingChoice filing{FilingChoice::Unfiled};
    bool chartsPropagated{};
};

struct ResearchEvidence {
    std::uint64_t stableSourceId{};
    ResearchField field{ResearchField::Geology};
    int points{};
    std::vector<std::string> tags;
    std::string description;
    KnowledgeObjectType objectType{KnowledgeObjectType::Fact};
    float confidence{1.0f};
    std::uint64_t provenanceStableId{};
};

struct ResearchUnlockDefinition {
    std::string id;
    ResearchField field{ResearchField::Geology};
    int pointsRequired{};
    std::vector<std::string> requiredEvidenceTags;
    ResearchUnlockKind kind{ResearchUnlockKind::Blueprint};
    float minimumEvidenceConfidence{};
};

struct ResourceKnowledge {
    std::string resourceId;
    float confidence{};

    friend bool operator==(const ResourceKnowledge&, const ResourceKnowledge&) = default;
};

struct PlanetChart {
    std::uint16_t planetIndex{};
    bool discovered{};
    bool surveyed{};
    bool planetClassKnown{};
    std::string planetClass;
    std::map<std::string, float> resourceConfidence;
    std::set<std::string> knownHazards;
    std::set<std::string> knownPoiFamilies;
};

struct SystemChart {
    std::uint32_t systemId{};
    bool charted{};
    bool positionKnown{};
    double xLy{};
    double yLy{};
    double zLy{};
    bool stationAccessKnown{};
    bool stationAccessible{};
    bool suspicionKnown{};
    float suspicion{};
    std::map<std::string, float> resourceConfidence;
    std::set<std::string> knownHazards;
    std::map<std::uint16_t, PlanetChart> planets;
};

struct MapFeature {
    std::uint64_t stableFeatureId{};
    MapLayer layer{MapLayer::Biome};
    std::uint32_t systemId{};
    std::uint16_t planetIndex{};
    bool hasSurfaceAddress{};
    SurfaceCellAddress surfaceAddress{};
    std::string label;
    float value{};
    float confidence{1.0f};
};

struct PlayerAnnotation {
    std::uint64_t stableAnnotationId{};
    std::uint32_t systemId{};
    std::uint16_t planetIndex{};
    bool hasSurfaceAddress{};
    SurfaceCellAddress surfaceAddress{};
    std::string text;
};

struct NeedQuery {
    std::optional<std::string> resourceId;
    float minimumResourceConfidence{};
    std::optional<std::string> planetClass;
    std::vector<std::string> excludedHazards;
    bool requireKnownAccessibleStation{};
    std::optional<float> maximumKnownSuspicion;
};

struct NeedMatch {
    std::uint32_t systemId{};
    std::optional<std::uint16_t> planetIndex;
    float resourceConfidence{};
    std::vector<std::string> evidence;
};

struct DiscoveryQuery {
    std::optional<ScanTargetKind> kind;
    std::optional<std::uint32_t> systemId;
    std::optional<std::uint16_t> planetIndex;
    std::optional<FilingChoice> filing;
    std::optional<std::string> requiredCatalogueTag;
    std::string text;
};

struct ResearchProgressReport {
    bool definitionFound{};
    bool unlocked{};
    ResearchField field{ResearchField::Geology};
    int qualifiedPoints{};
    int pointsRequired{};
    float minimumEvidenceConfidence{};
    std::size_t qualifyingEvidenceCount{};
    std::vector<std::string> missingEvidenceTags;
    std::vector<std::uint64_t> qualifyingSourceIds;
    std::string diagnostic;
};

struct RouteRequest {
    std::uint32_t originSystemId{};
    std::uint32_t destinationSystemId{};
    double jumpRangeLy{};
    double warpEfficiency{1.0};
    int availableFuelCells{};
};

enum class RouteFailure : std::uint8_t {
    None = 0,
    UnknownOrigin,
    UnknownDestination,
    PositionUnknown,
    InvalidShipProfile,
    Unreachable,
    InsufficientFuel
};

struct RouteEstimate {
    bool found{};
    RouteFailure failure{RouteFailure::None};
    std::vector<std::uint32_t> systems;
    double distanceLy{};
    int fuelCells{};
    int knownStationStops{};
    int unknownStationStops{};
    int unknownSuspicionStops{};
    float maximumKnownSuspicion{};
    int knownHazardTags{};
};

struct ScanOutcome {
    bool success{};
    ScanFailure failure{ScanFailure::None};
    int chargesConsumed{};
    bool newlyDiscovered{};
    ExplorationReward reward{};
    std::vector<std::string> newlyUnlockedResearch;
    std::string diagnostic;
};

struct FilingOutcome {
    bool success{};
    FilingChoice choice{FilingChoice::Unfiled};
    ExplorationReward reward{};
    bool chartsPropagated{};
    std::string diagnostic;
};

struct ExplorationTelemetry {
    std::uint64_t scanAttempts{};
    std::uint64_t successfulScans{};
    std::uint64_t newDiscoveries{};
    std::uint64_t duplicateScans{};
    std::uint64_t rejectedGrade{};
    std::uint64_t rejectedRange{};
    std::uint64_t rejectedCharge{};
    std::uint64_t filings{};
    std::uint64_t researchEvidenceAccepted{};
    std::uint64_t researchUnlocksGranted{};
    std::uint64_t needQueries{};
    std::uint64_t routeQueries{};
    std::uint64_t routeFailures{};
    std::uint64_t catalogueQueries{};
};

struct ExplorationDiagnostic {
    std::string operation;
    std::uint64_t subject{};
    std::string read;
    std::string decision;
    std::string changed;
};

class IExplorationMutationSink {
public:
    virtual ~IExplorationMutationSink() = default;
    virtual void addCredits(int amount) = 0;
    virtual void addFavor(float amount) = 0;
    virtual void addSystemSuspicion(std::uint32_t systemId, float amount) = 0;
    virtual void propagateCharts(std::uint64_t stableDiscoveryId) = 0;
};

class ExplorationService {
public:
    explicit ExplorationService(std::uint64_t worldSeed = 0);

    bool registerResearchUnlock(ResearchUnlockDefinition definition, std::string* error = nullptr);
    const std::vector<ResearchUnlockDefinition>& researchUnlockDefinitions() const { return researchUnlockDefinitions_; }

    ScanOutcome scan(const ScanTargetDescriptor& target, ScannerProfile& scanner, IExplorationMutationSink& sink);
    bool nameDiscovery(std::uint64_t stableTargetId, std::string name, std::string* error = nullptr);
    FilingOutcome fileDiscovery(std::uint64_t stableTargetId, FilingChoice choice, IExplorationMutationSink& sink);

    bool addResearchEvidence(ResearchEvidence evidence, std::vector<std::string>* newlyUnlocked = nullptr, std::string* error = nullptr);
    int researchPoints(ResearchField field) const;
    bool researchUnlocked(const std::string& id) const;
    const std::set<std::string>& unlockedResearch() const { return unlockedResearch_; }

    DiscoveryRecord* findDiscovery(std::uint64_t stableTargetId);
    const DiscoveryRecord* findDiscovery(std::uint64_t stableTargetId) const;
    const std::map<std::uint64_t, DiscoveryRecord>& discoveries() const { return discoveries_; }
    const std::map<std::uint64_t, ResearchEvidence>& researchEvidence() const { return researchEvidence_; }

    SystemChart& upsertSystemChart(std::uint32_t systemId);
    PlanetChart& upsertPlanetChart(std::uint32_t systemId, std::uint16_t planetIndex);
    const SystemChart* findSystemChart(std::uint32_t systemId) const;
    const std::map<std::uint32_t, SystemChart>& systemCharts() const { return systemCharts_; }

    std::uint64_t addMapFeature(MapFeature feature);
    bool removeMapFeature(std::uint64_t stableFeatureId);
    std::vector<MapFeature> mapFeatures(MapLayer layer, std::uint32_t systemId, std::optional<std::uint16_t> planetIndex = std::nullopt) const;
    void setLayerVisible(MapLayer layer, bool visible);
    bool layerVisible(MapLayer layer) const;

    std::uint64_t addAnnotation(PlayerAnnotation annotation);
    bool removeAnnotation(std::uint64_t stableAnnotationId);
    const std::map<std::uint64_t, PlayerAnnotation>& annotations() const { return annotations_; }

    std::vector<NeedMatch> need(const NeedQuery& query);
    std::vector<DiscoveryRecord> searchDiscoveries(const DiscoveryQuery& query);
    ResearchProgressReport researchProgress(const std::string& unlockId) const;
    RouteEstimate estimateRoute(const RouteRequest& request);

    const ExplorationTelemetry& telemetry() const { return telemetry_; }
    const std::vector<ExplorationDiagnostic>& diagnostics() const { return diagnostics_; }
    std::optional<ExplorationDiagnostic> lastDiagnostic() const;

    std::string serialize() const;
    bool restore(const std::string& text, std::string* error = nullptr);

private:
    std::uint64_t allocateStableId(std::uint64_t label, std::uint64_t& counter, const std::set<std::uint64_t>& occupied) const;
    void applyReward(const ExplorationReward& reward, std::uint32_t systemId, IExplorationMutationSink& sink) const;
    std::vector<std::string> evaluateResearchUnlocks();
    void pushDiagnostic(ExplorationDiagnostic diagnostic);

    std::uint64_t worldSeed_{};
    std::uint64_t annotationCounter_{1};
    std::uint64_t featureCounter_{1};
    std::map<std::uint64_t, DiscoveryRecord> discoveries_;
    std::map<std::uint64_t, ResearchEvidence> researchEvidence_;
    std::vector<ResearchUnlockDefinition> researchUnlockDefinitions_;
    std::set<std::string> unlockedResearch_;
    std::map<std::uint32_t, SystemChart> systemCharts_;
    std::map<std::uint64_t, MapFeature> mapFeatures_;
    std::map<std::uint64_t, PlayerAnnotation> annotations_;
    std::map<MapLayer, bool> layerVisibility_;
    ExplorationTelemetry telemetry_{};
    std::vector<ExplorationDiagnostic> diagnostics_;
};

} // namespace elysium
