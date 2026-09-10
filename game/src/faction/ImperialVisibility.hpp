#pragma once

#include <cstdint>
#include <map>
#include <span>
#include <vector>

namespace elysium::faction {

enum class VisibilityInfrastructureKind : std::uint8_t { InspectionBeacon, RegionalArchive };
enum class VisibilityInfrastructureState : std::uint8_t { Active, Disabled, Destroyed };
enum class InfrastructureFailure : std::uint8_t {
    None,
    Missing,
    RevisionConflict,
    OutOfRange,
    AlreadyDisabled,
    AlreadyActive,
    Destroyed,
    Protected,
    MissingDependency,
    Unauthorized,
    InvalidInput
};

struct VisibilityPosition { double x{}, y{}, z{}; };

struct VisibilityInfrastructure {
    std::uint64_t stableId{};
    std::uint64_t systemId{};
    std::uint64_t ownerId{};
    std::uint64_t jurisdictionId{};
    VisibilityInfrastructureKind kind{VisibilityInfrastructureKind::InspectionBeacon};
    VisibilityInfrastructureState state{VisibilityInfrastructureState::Active};
    VisibilityPosition position{};
    double sensorRange{};
    double inspectionStrength{};
    double minimumDeclaredVolume{};
    std::uint64_t knowledgeProviderRef{};
    std::uint64_t revision{1};
    bool protectedInfrastructure{};
};

struct DeclaredActivitySignal {
    std::uint64_t signalId{};
    std::uint64_t subjectId{};
    std::uint64_t systemId{};
    std::uint64_t activityType{};
    VisibilityPosition position{};
    double declaredVolume{};
    double signature{};
    std::uint64_t tick{};
};

struct VisibilityReport {
    std::uint64_t reportId{};
    std::uint64_t nodeId{};
    std::uint64_t subjectId{};
    std::uint64_t signalId{};
    std::uint64_t systemId{};
    double visibility{};
    double inspectionPressure{};
    std::uint64_t tick{};
};

struct KnowledgeFactView {
    std::uint64_t factId{};
    std::uint64_t subjectRef{};
    std::uint64_t provenanceRef{};
    double confidence{};
    std::uint64_t observedTick{};
};

struct RegionalArchiveView {
    std::uint64_t archiveId{};
    std::uint64_t systemId{};
    std::uint64_t infrastructureRevision{};
    std::vector<KnowledgeFactView> facts;
};

struct InfrastructureCommand {
    std::uint64_t commandId{};
    std::uint64_t stableId{};
    std::uint64_t expectedRevision{};
    std::uint64_t actorId{};
    double interactionDistance{};
    double maximumInteractionDistance{};
    bool authorized{};
    bool dependencyOnline{true};
};

struct InfrastructureResult {
    bool accepted{};
    InfrastructureFailure failure{InfrastructureFailure::None};
    std::uint64_t revision{};
};

struct ImperialVisibilitySnapshot {
    std::vector<VisibilityInfrastructure> infrastructure;
    std::vector<std::uint64_t> reportedReceiptKeys;
};

class ImperialVisibilityService {
public:
    bool publish(VisibilityInfrastructure infrastructure);
    [[nodiscard]] const VisibilityInfrastructure* find(std::uint64_t stableId) const;

    // Only caller-declared activity signals are inspected. This service never
    // scans arbitrary world/global simulation state.
    [[nodiscard]] std::vector<VisibilityReport> observe(std::span<const DeclaredActivitySignal> signals);

    InfrastructureResult disable(const InfrastructureCommand& command);
    InfrastructureResult restore(const InfrastructureCommand& command);
    InfrastructureResult destroy(const InfrastructureCommand& command);

    // Facts are supplied by the knowledge/filing owner. The archive is merely
    // a bounded projection and never owns or mutates map/knowledge truth.
    [[nodiscard]] RegionalArchiveView archiveView(std::uint64_t archiveId,
                                                  std::span<const KnowledgeFactView> sourceFacts,
                                                  std::size_t maximumFacts) const;

    [[nodiscard]] ImperialVisibilitySnapshot snapshot() const;
    bool restoreSnapshot(const ImperialVisibilitySnapshot& snapshot);

private:
    [[nodiscard]] InfrastructureResult validateTransition(const InfrastructureCommand& command,
                                                          bool enabling) const;
    [[nodiscard]] static std::uint64_t receiptKey(std::uint64_t nodeId,
                                                  std::uint64_t signalId) noexcept;
    [[nodiscard]] static std::uint64_t reportId(std::uint64_t nodeId,
                                                std::uint64_t signalId) noexcept;

    std::map<std::uint64_t, VisibilityInfrastructure> infrastructure_;
    std::map<std::uint64_t, bool> reportedReceipts_;
};

} // namespace elysium::faction
