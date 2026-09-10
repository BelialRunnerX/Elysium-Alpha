// Intended function: imported world implementation for SurfaceItems; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "world/PlanetSurface.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium {

// Fourth Edition Parts 13-14: bulk matter stays aggregated until some gameplay
// rule needs individual identity. Stable IDs are save/network identities and are
// never entt::entity values.
enum class SurfaceItemIdentityTier : std::uint8_t {
    BulkAggregate = 0,
    Stack = 1,
    Individual = 2,
    NamedOwned = 3,
    Artifact = 4
};

enum class SurfaceOwnerKind : std::uint8_t {
    None = 0,
    Player = 1,
    Citizen = 2,
    Institution = 3,
    Squad = 4,
    Settlement = 5,
    Faction = 6
};

enum class SurfaceItemLocationKind : std::uint8_t {
    Loose = 0,
    Stockpile = 1,
    Carried = 2,
    Container = 3,
    Equipped = 4,
    TradeManifest = 5
};

enum class SurfaceReservationKind : std::uint8_t {
    Hauling = 0,
    JobInput = 1,
    Tool = 2,
    Equipment = 3,
    Construction = 4,
    Trade = 5
};

enum class SurfaceReservationInvalidationReason : std::uint8_t {
    Released = 0,
    ResourceMoved = 1,
    ResourceDestroyed = 2,
    ResourceChanged = 3,
    LeaseExpired = 4,
    OwnerCancelled = 5
};

enum class SurfaceHaulMode : std::uint8_t {
    Hand = 0,
    CartTrolley = 1,
    Conveyor = 2,
    Drone = 3,
    CargoRail = 4,
    Ship = 5
};

enum class SurfaceHaulState : std::uint8_t {
    Reserved = 0,
    InTransit = 1,
    Completed = 2,
    Blocked = 3,
    Cancelled = 4
};

enum class SurfaceHaulFailure : std::uint8_t {
    None = 0,
    SourceMissing = 1,
    DestinationMissing = 2,
    NoAcceptedItem = 3,
    DestinationFull = 4,
    ReservationConflict = 5,
    ContainerMissing = 6,
    ContainerFull = 7,
    UnsupportedTransport = 8,
    ResourceInvalidated = 9,
    LinkPolicyRejected = 10
};

struct SurfaceItemKey {
    int itemId{};                  // stable ContentId-compatible integer seam
    int materialId{};              // material registry ID, 0 = definition default
    std::uint16_t quality{};       // deterministic craft-quality bucket
    std::uint16_t conditionPermille{1000}; // 0..1000; exact bucket for aggregation
    std::uint32_t contaminationMask{};

    friend bool operator==(const SurfaceItemKey&, const SurfaceItemKey&) = default;
};

struct SurfaceOwnershipClaim {
    SurfaceOwnerKind kind{SurfaceOwnerKind::None};
    std::uint64_t ownerStableId{};
    std::uint32_t acquisitionBasis{}; // data-driven/event/content code

    bool owned() const { return kind != SurfaceOwnerKind::None && ownerStableId != 0; }
    friend bool operator==(const SurfaceOwnershipClaim&, const SurfaceOwnershipClaim&) = default;
};

struct SurfaceItemProvenance {
    std::uint64_t makerStableId{};
    std::uint64_t siteStableId{};
    std::uint64_t sourceStableId{};
    std::uint64_t createdTick{};
    std::uint32_t sourceTag{};
    bool preserve{}; // if true the object may never silently reaggregate

    friend bool operator==(const SurfaceItemProvenance&, const SurfaceItemProvenance&) = default;
};

struct SurfaceItemLocation {
    SurfaceItemLocationKind kind{SurfaceItemLocationKind::Loose};
    SurfaceCellAddress cell{};
    std::uint64_t holderStableId{}; // stockpile/actor/container/manifest stable ID

    friend bool operator==(const SurfaceItemLocation&, const SurfaceItemLocation&) = default;
};

// ECS-facing persistent item component envelope. The current standalone keeps
// these in a deterministic local ledger so headless tests do not require EnTT;
// the fields are deliberately component-shaped for translation into the main
// registry without changing save identity.
struct SurfaceItemRecord {
    std::uint64_t stableId{};
    SurfaceItemIdentityTier identityTier{SurfaceItemIdentityTier::Stack};
    SurfaceItemKey key{};
    int quantity{};
    SurfaceItemLocation location{};
    SurfaceOwnershipClaim ownership{};
    SurfaceItemProvenance provenance{};
    std::uint64_t artifactStableId{}; // seam only; narrative generation is out of scope

    friend bool operator==(const SurfaceItemRecord&, const SurfaceItemRecord&) = default;
};

struct SurfaceBulkBucket {
    SurfaceItemKey key{};
    int quantity{};

    friend bool operator==(const SurfaceBulkBucket&, const SurfaceBulkBucket&) = default;
};

struct SurfaceStockpileFilter {
    std::vector<int> allowedItemIds;      // empty = any
    std::vector<int> allowedMaterialIds;  // empty = any
    std::uint16_t minimumQuality{};
    std::uint16_t maximumQuality{65535};
    std::uint16_t minimumConditionPermille{};
    std::uint32_t rejectedContaminationMask{};
    bool allowOwnedItems{};

    friend bool operator==(const SurfaceStockpileFilter&, const SurfaceStockpileFilter&) = default;
};

struct SurfaceStockpileRecord {
    std::uint64_t stableId{};
    std::vector<SurfaceCellAddress> cells; // bounded explicit spatial mask; no dense planet grid
    SurfaceStockpileFilter filter{};
    int priority{};
    int maxUnits{4096};
    std::vector<std::uint64_t> giveToLinks;
    std::vector<std::uint64_t> takeFromLinks;
    std::uint64_t overflowStockpileStableId{};
    bool allowVehicles{true};
    bool sealedContainers{};
    std::uint32_t acceptedHazardMask{0xFFFFFFFFu};
    std::vector<SurfaceBulkBucket> bulk;

    friend bool operator==(const SurfaceStockpileRecord&, const SurfaceStockpileRecord&) = default;
};

struct SurfaceReservationRecord {
    std::uint64_t stableId{};
    SurfaceReservationKind kind{SurfaceReservationKind::JobInput};
    std::uint64_t ownerStableId{};
    std::uint64_t dependentStableId{}; // e.g. Job StableId; zero when not applicable
    std::uint64_t resourceStableId{};
    int quantity{};
    std::uint64_t expiresTick{}; // zero = explicit release only

    friend bool operator==(const SurfaceReservationRecord&, const SurfaceReservationRecord&) = default;
};

struct SurfaceReservationInvalidation {
    std::uint64_t reservationStableId{};
    std::uint64_t ownerStableId{};
    std::uint64_t dependentStableId{};
    std::uint64_t resourceStableId{};
    SurfaceReservationInvalidationReason reason{SurfaceReservationInvalidationReason::ResourceChanged};

    friend bool operator==(const SurfaceReservationInvalidation&, const SurfaceReservationInvalidation&) = default;
};

struct SurfaceHaulContainer {
    std::uint64_t stableId{};
    std::uint64_t ownerStableId{};
    int capacityUnits{32};
    std::vector<std::uint64_t> cargoItemStableIds;

    friend bool operator==(const SurfaceHaulContainer&, const SurfaceHaulContainer&) = default;
};

struct SurfaceHaulTask {
    std::uint64_t stableId{};
    std::uint64_t workerStableId{};
    SurfaceHaulMode mode{SurfaceHaulMode::Hand};
    SurfaceHaulState state{SurfaceHaulState::Reserved};
    SurfaceHaulFailure failure{SurfaceHaulFailure::None};
    std::uint64_t sourceStockpileStableId{};
    std::uint64_t destinationStockpileStableId{};
    std::uint64_t carrierStableId{}; // cart/trolley container StableId; worker for hand
    std::uint64_t itemStableId{};
    std::uint64_t reservationStableId{};

    friend bool operator==(const SurfaceHaulTask&, const SurfaceHaulTask&) = default;
};

// Interface packet for routing systems owned by other agents. This ledger does
// not execute conveyors, drones, cargo rail, or ships; it can prepare a stable-
// ID transfer request and later accept an authoritative delivery commit.
struct SurfaceExternalHaulRequest {
    SurfaceHaulMode mode{SurfaceHaulMode::Conveyor};
    std::uint64_t requestStableId{};
    std::uint64_t itemStableId{};
    std::uint64_t reservationStableId{};
    std::uint64_t sourceStockpileStableId{};
    std::uint64_t destinationStockpileStableId{};
    std::uint64_t transportStableId{};

    friend bool operator==(const SurfaceExternalHaulRequest&, const SurfaceExternalHaulRequest&) = default;
};

struct SurfaceItemLedgerSnapshot {
    std::uint32_t schemaVersion{1};
    std::uint64_t worldSeed{};
    std::uint64_t nextSerial{1};
    std::vector<SurfaceItemRecord> items;
    std::vector<SurfaceStockpileRecord> stockpiles;
    std::vector<SurfaceReservationRecord> reservations;
    std::vector<SurfaceHaulContainer> containers;
    std::vector<SurfaceHaulTask> haulTasks;

    friend bool operator==(const SurfaceItemLedgerSnapshot&, const SurfaceItemLedgerSnapshot&) = default;
};

class SurfaceItemLedger {
public:
    explicit SurfaceItemLedger(std::uint64_t worldSeed = 0);

    std::uint64_t createStockpile(std::vector<SurfaceCellAddress> cells,
                                  SurfaceStockpileFilter filter = {},
                                  int priority = 0,
                                  int maxUnits = 4096);
    bool restoreStockpile(const SurfaceStockpileRecord& stockpile);
    SurfaceStockpileRecord* findStockpile(std::uint64_t stableId);
    const SurfaceStockpileRecord* findStockpile(std::uint64_t stableId) const;
    bool setStockpilePriority(std::uint64_t stableId, int priority);
    bool setStockpileLinks(std::uint64_t stableId,
                           std::vector<std::uint64_t> giveTo,
                           std::vector<std::uint64_t> takeFrom,
                           std::uint64_t overflow = 0);

    bool addBulk(std::uint64_t stockpileStableId, SurfaceItemKey key, int quantity);
    int bulkCount(std::uint64_t stockpileStableId, const SurfaceItemKey& key) const;
    int stockpileUnits(std::uint64_t stockpileStableId) const;
    std::uint64_t bestDestination(const SurfaceItemRecord& item,
                                  std::uint64_t sourceStockpileStableId = 0) const;
    std::uint64_t bestDestination(const SurfaceItemKey& key,
                                  SurfaceOwnershipClaim ownership = {},
                                  std::uint64_t sourceStockpileStableId = 0) const;
    // Deterministic source selection drains lower-priority stockpiles first,
    // then StableId. This preserves high-priority reserves while avoiding
    // unordered-container or worker-order dependence.
    std::uint64_t bestSource(const SurfaceItemKey& key,
                             int quantity,
                             std::uint64_t destinationStockpileStableId = 0) const;
    // Apply a stockpile's explicit overflow chain. Cycles or unresolved links
    // fail closed instead of scanning the planet for another destination.
    std::uint64_t resolveDestination(std::uint64_t preferredStockpileStableId,
                                     const SurfaceItemKey& key,
                                     SurfaceOwnershipClaim ownership = {},
                                     int quantity = 1,
                                     std::uint64_t sourceStockpileStableId = 0) const;

    std::uint64_t promoteFromBulk(std::uint64_t stockpileStableId,
                                  const SurfaceItemKey& key,
                                  int quantity,
                                  SurfaceItemIdentityTier minimumTier = SurfaceItemIdentityTier::Stack,
                                  SurfaceItemProvenance provenance = {});
    bool tryReaggregate(std::uint64_t itemStableId, std::uint64_t stockpileStableId);
    SurfaceItemRecord* findItem(std::uint64_t stableId);
    const SurfaceItemRecord* findItem(std::uint64_t stableId) const;
    bool setOwnership(std::uint64_t itemStableId, SurfaceOwnershipClaim ownership);
    bool markArtifact(std::uint64_t itemStableId, std::uint64_t artifactStableId);
    bool moveItem(std::uint64_t itemStableId,
                  SurfaceItemLocation destination,
                  std::uint64_t authorizingReservationStableId = 0);
    bool destroyItem(std::uint64_t itemStableId);

    std::uint64_t reserveItem(std::uint64_t resourceStableId,
                              std::uint64_t ownerStableId,
                              SurfaceReservationKind kind,
                              int quantity,
                              std::uint64_t expiresTick = 0,
                              std::uint64_t dependentStableId = 0);
    bool releaseReservation(std::uint64_t reservationStableId,
                            SurfaceReservationInvalidationReason reason = SurfaceReservationInvalidationReason::Released,
                            bool emitDiagnostic = false);
    void expireReservations(std::uint64_t currentTick);
    const SurfaceReservationRecord* findReservation(std::uint64_t stableId) const;
    const SurfaceReservationRecord* reservationForResource(std::uint64_t resourceStableId) const;
    std::vector<SurfaceReservationInvalidation> consumeReservationInvalidations();

    std::uint64_t createHaulContainer(std::uint64_t ownerStableId, int capacityUnits = 32);
    const SurfaceHaulContainer* findHaulContainer(std::uint64_t stableId) const;

    std::uint64_t beginHaul(std::uint64_t workerStableId,
                            std::uint64_t sourceStockpileStableId,
                            std::uint64_t destinationStockpileStableId,
                            const SurfaceItemKey& key,
                            int quantity,
                            SurfaceHaulMode mode,
                            std::uint64_t currentTick,
                            std::uint64_t leaseTicks = 120,
                            std::uint64_t carrierStableId = 0,
                            std::uint64_t dependentStableId = 0);
    bool completeHaul(std::uint64_t haulTaskStableId);
    bool cancelHaul(std::uint64_t haulTaskStableId);
    SurfaceHaulTask* findHaulTask(std::uint64_t stableId);
    const SurfaceHaulTask* findHaulTask(std::uint64_t stableId) const;

    std::optional<SurfaceExternalHaulRequest> prepareExternalHaul(
        std::uint64_t workerOrControllerStableId,
        std::uint64_t transportStableId,
        std::uint64_t sourceStockpileStableId,
        std::uint64_t destinationStockpileStableId,
        const SurfaceItemKey& key,
        int quantity,
        SurfaceHaulMode mode,
        std::uint64_t currentTick,
        std::uint64_t leaseTicks = 120,
        std::uint64_t dependentStableId = 0);
    bool commitExternalDelivery(const SurfaceExternalHaulRequest& request);

    int totalQuantity(int itemId) const;
    SurfaceItemLedgerSnapshot snapshot() const;
    bool restore(const SurfaceItemLedgerSnapshot& snapshot, std::string* error = nullptr);

    const std::vector<SurfaceItemRecord>& items() const { return items_; }
    const std::vector<SurfaceStockpileRecord>& stockpiles() const { return stockpiles_; }
    const std::vector<SurfaceReservationRecord>& reservations() const { return reservations_; }
    const std::vector<SurfaceHaulContainer>& haulContainers() const { return containers_; }
    const std::vector<SurfaceHaulTask>& haulTasks() const { return haulTasks_; }

    static bool stockpileAccepts(const SurfaceStockpileRecord& stockpile,
                                 const SurfaceItemKey& key,
                                 const SurfaceOwnershipClaim& ownership = {});
    static const char* haulFailureName(SurfaceHaulFailure failure);
    static const char* reservationInvalidationName(SurfaceReservationInvalidationReason reason);

private:
    std::uint64_t worldSeed_{};
    std::uint64_t nextSerial_{1};
    std::vector<SurfaceItemRecord> items_;
    std::vector<SurfaceStockpileRecord> stockpiles_;
    std::vector<SurfaceReservationRecord> reservations_;
    std::vector<SurfaceReservationInvalidation> invalidations_;
    std::vector<SurfaceHaulContainer> containers_;
    std::vector<SurfaceHaulTask> haulTasks_;

    std::uint64_t allocateStableId();
    bool stableIdInUse(std::uint64_t stableId) const;
    static bool validKey(const SurfaceItemKey& key);
    static bool itemKeyLess(const SurfaceItemKey& a, const SurfaceItemKey& b);
    static void normalizeIds(std::vector<std::uint64_t>& ids);
    static void normalizeBulk(SurfaceStockpileRecord& stockpile);
    static bool canSafelyAggregate(const SurfaceItemRecord& item);
    static int containerUnits(const SurfaceHaulContainer& container,
                              const std::vector<SurfaceItemRecord>& items);
    int stockpileIdentityUnits(std::uint64_t stableId) const;
    bool stockpileCanAcceptUnits(const SurfaceStockpileRecord& stockpile, int quantity) const;
    bool linksAllow(std::uint64_t sourceStockpileStableId, std::uint64_t destinationStockpileStableId) const;
    void invalidateReservationsForResource(std::uint64_t resourceStableId,
                                           SurfaceReservationInvalidationReason reason,
                                           std::uint64_t exceptReservationStableId = 0);
    void setHaulFailure(SurfaceHaulTask& task, SurfaceHaulFailure failure);
    void blockHaulsForReservation(std::uint64_t reservationStableId,
                                  SurfaceHaulFailure failure = SurfaceHaulFailure::ResourceInvalidated);
};

} // namespace elysium
