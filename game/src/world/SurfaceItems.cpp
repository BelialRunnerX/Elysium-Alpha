// Intended function: imported world implementation for SurfaceItems; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/SurfaceItems.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace elysium {
namespace {

bool addressLess(const SurfaceCellAddress& a, const SurfaceCellAddress& b) {
    if (a.face != b.face) return static_cast<int>(a.face) < static_cast<int>(b.face);
    if (a.u != b.u) return a.u < b.u;
    if (a.v != b.v) return a.v < b.v;
    return a.radial < b.radial;
}

bool contains(const std::vector<int>& values, int value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

void setError(std::string* error, const char* message) {
    if (error) *error = message;
}

} // namespace

SurfaceItemLedger::SurfaceItemLedger(std::uint64_t worldSeed) : worldSeed_(worldSeed) {}

bool SurfaceItemLedger::validKey(const SurfaceItemKey& key) {
    return key.itemId > 0 && key.conditionPermille <= 1000;
}

bool SurfaceItemLedger::itemKeyLess(const SurfaceItemKey& a, const SurfaceItemKey& b) {
    if (a.itemId != b.itemId) return a.itemId < b.itemId;
    if (a.materialId != b.materialId) return a.materialId < b.materialId;
    if (a.quality != b.quality) return a.quality < b.quality;
    if (a.conditionPermille != b.conditionPermille) return a.conditionPermille < b.conditionPermille;
    return a.contaminationMask < b.contaminationMask;
}

void SurfaceItemLedger::normalizeIds(std::vector<std::uint64_t>& ids) {
    ids.erase(std::remove(ids.begin(), ids.end(), 0), ids.end());
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
}

void SurfaceItemLedger::normalizeBulk(SurfaceStockpileRecord& stockpile) {
    stockpile.bulk.erase(std::remove_if(stockpile.bulk.begin(), stockpile.bulk.end(), [](const auto& bucket) {
        return bucket.quantity <= 0 || !validKey(bucket.key);
    }), stockpile.bulk.end());
    std::sort(stockpile.bulk.begin(), stockpile.bulk.end(), [](const auto& a, const auto& b) {
        return itemKeyLess(a.key, b.key);
    });
    std::vector<SurfaceBulkBucket> merged;
    for (const auto& bucket : stockpile.bulk) {
        if (!merged.empty() && merged.back().key == bucket.key) {
            const long long sum = static_cast<long long>(merged.back().quantity) + bucket.quantity;
            merged.back().quantity = static_cast<int>(std::min<long long>(sum, std::numeric_limits<int>::max()));
        } else {
            merged.push_back(bucket);
        }
    }
    stockpile.bulk = std::move(merged);
}

bool SurfaceItemLedger::stableIdInUse(std::uint64_t stableId) const {
    if (stableId == 0) return true;
    const auto has = [stableId](const auto& values) {
        return std::any_of(values.begin(), values.end(), [stableId](const auto& value) { return value.stableId == stableId; });
    };
    return has(items_) || has(stockpiles_) || has(reservations_) || has(containers_) || has(haulTasks_);
}

std::uint64_t SurfaceItemLedger::allocateStableId() {
    for (;;) {
        const std::uint64_t id = mix64(worldSeed_ ^ 0x4954454D4C454447ULL ^ nextSerial_++); // ITEMLEDG
        if (!stableIdInUse(id)) return id;
    }
}

std::uint64_t SurfaceItemLedger::createStockpile(std::vector<SurfaceCellAddress> cells,
                                                 SurfaceStockpileFilter filter,
                                                 int priority,
                                                 int maxUnits) {
    if (maxUnits <= 0 || cells.empty()) return 0;
    std::sort(cells.begin(), cells.end(), addressLess);
    cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
    std::sort(filter.allowedItemIds.begin(), filter.allowedItemIds.end());
    filter.allowedItemIds.erase(std::unique(filter.allowedItemIds.begin(), filter.allowedItemIds.end()), filter.allowedItemIds.end());
    std::sort(filter.allowedMaterialIds.begin(), filter.allowedMaterialIds.end());
    filter.allowedMaterialIds.erase(std::unique(filter.allowedMaterialIds.begin(), filter.allowedMaterialIds.end()), filter.allowedMaterialIds.end());
    SurfaceStockpileRecord record{};
    record.stableId = allocateStableId();
    record.cells = std::move(cells);
    record.filter = std::move(filter);
    record.priority = priority;
    record.maxUnits = maxUnits;
    stockpiles_.push_back(record);
    std::sort(stockpiles_.begin(), stockpiles_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return record.stableId;
}

bool SurfaceItemLedger::restoreStockpile(const SurfaceStockpileRecord& stockpile) {
    if (stockpile.stableId == 0 || stableIdInUse(stockpile.stableId) || stockpile.maxUnits <= 0 || stockpile.cells.empty()) return false;
    SurfaceStockpileRecord copy = stockpile;
    std::sort(copy.cells.begin(), copy.cells.end(), addressLess);
    copy.cells.erase(std::unique(copy.cells.begin(), copy.cells.end()), copy.cells.end());
    normalizeIds(copy.giveToLinks);
    normalizeIds(copy.takeFromLinks);
    std::sort(copy.filter.allowedItemIds.begin(), copy.filter.allowedItemIds.end());
    copy.filter.allowedItemIds.erase(std::unique(copy.filter.allowedItemIds.begin(), copy.filter.allowedItemIds.end()), copy.filter.allowedItemIds.end());
    std::sort(copy.filter.allowedMaterialIds.begin(), copy.filter.allowedMaterialIds.end());
    copy.filter.allowedMaterialIds.erase(std::unique(copy.filter.allowedMaterialIds.begin(), copy.filter.allowedMaterialIds.end()), copy.filter.allowedMaterialIds.end());
    normalizeBulk(copy);
    long long restoredUnits = 0;
    for (const auto& bucket : copy.bulk) restoredUnits += bucket.quantity;
    if (restoredUnits > copy.maxUnits) return false;
    stockpiles_.push_back(std::move(copy));
    std::sort(stockpiles_.begin(), stockpiles_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return true;
}

SurfaceStockpileRecord* SurfaceItemLedger::findStockpile(std::uint64_t stableId) {
    const auto it = std::lower_bound(stockpiles_.begin(), stockpiles_.end(), stableId,
                                     [](const auto& value, std::uint64_t id) { return value.stableId < id; });
    return it != stockpiles_.end() && it->stableId == stableId ? &*it : nullptr;
}

const SurfaceStockpileRecord* SurfaceItemLedger::findStockpile(std::uint64_t stableId) const {
    const auto it = std::lower_bound(stockpiles_.begin(), stockpiles_.end(), stableId,
                                     [](const auto& value, std::uint64_t id) { return value.stableId < id; });
    return it != stockpiles_.end() && it->stableId == stableId ? &*it : nullptr;
}

bool SurfaceItemLedger::setStockpilePriority(std::uint64_t stableId, int priority) {
    auto* stockpile = findStockpile(stableId);
    if (!stockpile) return false;
    stockpile->priority = priority;
    return true;
}

bool SurfaceItemLedger::setStockpileLinks(std::uint64_t stableId,
                                          std::vector<std::uint64_t> giveTo,
                                          std::vector<std::uint64_t> takeFrom,
                                          std::uint64_t overflow) {
    auto* stockpile = findStockpile(stableId);
    if (!stockpile) return false;
    normalizeIds(giveTo);
    normalizeIds(takeFrom);
    if (std::find(giveTo.begin(), giveTo.end(), stableId) != giveTo.end() ||
        std::find(takeFrom.begin(), takeFrom.end(), stableId) != takeFrom.end()) return false;
    for (const auto id : giveTo) if (!findStockpile(id)) return false;
    for (const auto id : takeFrom) if (!findStockpile(id)) return false;
    if (overflow != 0 && (!findStockpile(overflow) || overflow == stableId)) return false;
    stockpile->giveToLinks = std::move(giveTo);
    stockpile->takeFromLinks = std::move(takeFrom);
    stockpile->overflowStockpileStableId = overflow;
    return true;
}

bool SurfaceItemLedger::stockpileAccepts(const SurfaceStockpileRecord& stockpile,
                                         const SurfaceItemKey& key,
                                         const SurfaceOwnershipClaim& ownership) {
    if (!validKey(key)) return false;
    const auto& filter = stockpile.filter;
    if (!filter.allowedItemIds.empty() && !contains(filter.allowedItemIds, key.itemId)) return false;
    if (!filter.allowedMaterialIds.empty() && !contains(filter.allowedMaterialIds, key.materialId)) return false;
    if (key.quality < filter.minimumQuality || key.quality > filter.maximumQuality) return false;
    if (key.conditionPermille < filter.minimumConditionPermille) return false;
    if ((key.contaminationMask & filter.rejectedContaminationMask) != 0) return false;
    if (ownership.owned() && !filter.allowOwnedItems) return false;
    if ((key.contaminationMask & ~stockpile.acceptedHazardMask) != 0) return false;
    return true;
}

int SurfaceItemLedger::stockpileIdentityUnits(std::uint64_t stableId) const {
    int total = 0;
    for (const auto& item : items_) {
        if (item.location.kind == SurfaceItemLocationKind::Stockpile && item.location.holderStableId == stableId)
            total += item.quantity;
    }
    return total;
}

int SurfaceItemLedger::stockpileUnits(std::uint64_t stableId) const {
    const auto* stockpile = findStockpile(stableId);
    if (!stockpile) return 0;
    long long total = stockpileIdentityUnits(stableId);
    for (const auto& bucket : stockpile->bulk) total += bucket.quantity;
    return static_cast<int>(std::min<long long>(total, std::numeric_limits<int>::max()));
}

bool SurfaceItemLedger::stockpileCanAcceptUnits(const SurfaceStockpileRecord& stockpile, int quantity) const {
    if (quantity <= 0) return false;
    return stockpileUnits(stockpile.stableId) <= stockpile.maxUnits - quantity;
}

bool SurfaceItemLedger::addBulk(std::uint64_t stockpileStableId, SurfaceItemKey key, int quantity) {
    auto* stockpile = findStockpile(stockpileStableId);
    if (!stockpile || quantity <= 0 || !stockpileAccepts(*stockpile, key) || !stockpileCanAcceptUnits(*stockpile, quantity)) return false;
    auto it = std::lower_bound(stockpile->bulk.begin(), stockpile->bulk.end(), key,
                               [](const auto& bucket, const auto& candidate) { return itemKeyLess(bucket.key, candidate); });
    if (it != stockpile->bulk.end() && it->key == key) it->quantity += quantity;
    else stockpile->bulk.insert(it, SurfaceBulkBucket{key, quantity});
    return true;
}

int SurfaceItemLedger::bulkCount(std::uint64_t stockpileStableId, const SurfaceItemKey& key) const {
    const auto* stockpile = findStockpile(stockpileStableId);
    if (!stockpile) return 0;
    const auto it = std::lower_bound(stockpile->bulk.begin(), stockpile->bulk.end(), key,
                                     [](const auto& bucket, const auto& candidate) { return itemKeyLess(bucket.key, candidate); });
    return it != stockpile->bulk.end() && it->key == key ? it->quantity : 0;
}

bool SurfaceItemLedger::linksAllow(std::uint64_t sourceStockpileStableId, std::uint64_t destinationStockpileStableId) const {
    if (sourceStockpileStableId == 0) return true;
    const auto* source = findStockpile(sourceStockpileStableId);
    const auto* destination = findStockpile(destinationStockpileStableId);
    if (!source || !destination) return false;
    if (!source->giveToLinks.empty() && !std::binary_search(source->giveToLinks.begin(), source->giveToLinks.end(), destinationStockpileStableId)) return false;
    if (!destination->takeFromLinks.empty() && !std::binary_search(destination->takeFromLinks.begin(), destination->takeFromLinks.end(), sourceStockpileStableId)) return false;
    return true;
}

std::uint64_t SurfaceItemLedger::bestDestination(const SurfaceItemRecord& item,
                                                 std::uint64_t sourceStockpileStableId) const {
    std::uint64_t best = 0;
    int bestPriority = std::numeric_limits<int>::min();
    for (const auto& stockpile : stockpiles_) {
        if (stockpile.stableId == sourceStockpileStableId ||
            !stockpileAccepts(stockpile, item.key, item.ownership) ||
            !stockpileCanAcceptUnits(stockpile, item.quantity) ||
            !linksAllow(sourceStockpileStableId, stockpile.stableId)) continue;
        if (stockpile.priority > bestPriority || (stockpile.priority == bestPriority && (best == 0 || stockpile.stableId < best))) {
            bestPriority = stockpile.priority;
            best = stockpile.stableId;
        }
    }
    return best;
}

std::uint64_t SurfaceItemLedger::bestDestination(const SurfaceItemKey& key,
                                                 SurfaceOwnershipClaim ownership,
                                                 std::uint64_t sourceStockpileStableId) const {
    SurfaceItemRecord probe{};
    probe.key = key;
    probe.quantity = 1;
    probe.ownership = ownership;
    return bestDestination(probe, sourceStockpileStableId);
}


std::uint64_t SurfaceItemLedger::bestSource(const SurfaceItemKey& key,
                                            int quantity,
                                            std::uint64_t destinationStockpileStableId) const {
    if (quantity <= 0 || !validKey(key)) return 0;
    std::uint64_t best = 0;
    int bestPriority = std::numeric_limits<int>::max();
    for (const auto& stockpile : stockpiles_) {
        if (stockpile.stableId == destinationStockpileStableId || bulkCount(stockpile.stableId, key) < quantity) continue;
        if (destinationStockpileStableId != 0 && !linksAllow(stockpile.stableId, destinationStockpileStableId)) continue;
        if (stockpile.priority < bestPriority || (stockpile.priority == bestPriority && (best == 0 || stockpile.stableId < best))) {
            bestPriority = stockpile.priority;
            best = stockpile.stableId;
        }
    }
    return best;
}

std::uint64_t SurfaceItemLedger::resolveDestination(std::uint64_t preferredStockpileStableId,
                                                    const SurfaceItemKey& key,
                                                    SurfaceOwnershipClaim ownership,
                                                    int quantity,
                                                    std::uint64_t sourceStockpileStableId) const {
    if (preferredStockpileStableId == 0 || quantity <= 0) return 0;
    std::vector<std::uint64_t> visited;
    std::uint64_t current = preferredStockpileStableId;
    while (current != 0) {
        if (std::find(visited.begin(), visited.end(), current) != visited.end()) return 0;
        visited.push_back(current);
        const auto* stockpile = findStockpile(current);
        if (!stockpile) return 0;
        if (stockpile->stableId != sourceStockpileStableId &&
            stockpileAccepts(*stockpile, key, ownership) &&
            stockpileCanAcceptUnits(*stockpile, quantity) &&
            linksAllow(sourceStockpileStableId, stockpile->stableId)) return stockpile->stableId;
        current = stockpile->overflowStockpileStableId;
    }
    return 0;
}

std::uint64_t SurfaceItemLedger::promoteFromBulk(std::uint64_t stockpileStableId,
                                                 const SurfaceItemKey& key,
                                                 int quantity,
                                                 SurfaceItemIdentityTier minimumTier,
                                                 SurfaceItemProvenance provenance) {
    auto* stockpile = findStockpile(stockpileStableId);
    if (!stockpile || quantity <= 0 || minimumTier == SurfaceItemIdentityTier::BulkAggregate) minimumTier = SurfaceItemIdentityTier::Stack;
    if (!stockpile || quantity <= 0) return 0;
    auto it = std::lower_bound(stockpile->bulk.begin(), stockpile->bulk.end(), key,
                               [](const auto& bucket, const auto& candidate) { return itemKeyLess(bucket.key, candidate); });
    if (it == stockpile->bulk.end() || !(it->key == key) || it->quantity < quantity) return 0;
    it->quantity -= quantity;
    if (it->quantity == 0) stockpile->bulk.erase(it);

    SurfaceItemRecord item{};
    item.stableId = allocateStableId();
    item.identityTier = std::max(minimumTier, SurfaceItemIdentityTier::Stack);
    item.key = key;
    item.quantity = quantity;
    item.location = {SurfaceItemLocationKind::Stockpile, stockpile->cells.front(), stockpileStableId};
    item.provenance = provenance;
    if (provenance.preserve && item.identityTier < SurfaceItemIdentityTier::Individual)
        item.identityTier = SurfaceItemIdentityTier::Individual;
    items_.push_back(item);
    std::sort(items_.begin(), items_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return item.stableId;
}

SurfaceItemRecord* SurfaceItemLedger::findItem(std::uint64_t stableId) {
    const auto it = std::lower_bound(items_.begin(), items_.end(), stableId,
                                     [](const auto& item, std::uint64_t id) { return item.stableId < id; });
    return it != items_.end() && it->stableId == stableId ? &*it : nullptr;
}

const SurfaceItemRecord* SurfaceItemLedger::findItem(std::uint64_t stableId) const {
    const auto it = std::lower_bound(items_.begin(), items_.end(), stableId,
                                     [](const auto& item, std::uint64_t id) { return item.stableId < id; });
    return it != items_.end() && it->stableId == stableId ? &*it : nullptr;
}

bool SurfaceItemLedger::canSafelyAggregate(const SurfaceItemRecord& item) {
    return item.identityTier <= SurfaceItemIdentityTier::Stack &&
           !item.ownership.owned() &&
           !item.provenance.preserve &&
           item.artifactStableId == 0 &&
           item.quantity > 0 &&
           validKey(item.key);
}

bool SurfaceItemLedger::tryReaggregate(std::uint64_t itemStableId, std::uint64_t stockpileStableId) {
    auto* item = findItem(itemStableId);
    auto* stockpile = findStockpile(stockpileStableId);
    if (!item || !stockpile || !canSafelyAggregate(*item) ||
        !stockpileAccepts(*stockpile, item->key, item->ownership) ||
        item->location.kind != SurfaceItemLocationKind::Stockpile ||
        item->location.holderStableId != stockpileStableId) return false;
    if (reservationForResource(itemStableId)) return false;
    const auto key = item->key;
    const int quantity = item->quantity;
    const auto it = std::lower_bound(stockpile->bulk.begin(), stockpile->bulk.end(), key,
                                     [](const auto& bucket, const auto& candidate) { return itemKeyLess(bucket.key, candidate); });
    if (it != stockpile->bulk.end() && it->key == key) it->quantity += quantity;
    else stockpile->bulk.insert(it, SurfaceBulkBucket{key, quantity});
    items_.erase(std::remove_if(items_.begin(), items_.end(), [itemStableId](const auto& value) { return value.stableId == itemStableId; }), items_.end());
    return true;
}

bool SurfaceItemLedger::setOwnership(std::uint64_t itemStableId, SurfaceOwnershipClaim ownership) {
    auto* item = findItem(itemStableId);
    if (!item) return false;
    if ((ownership.kind == SurfaceOwnerKind::None) != (ownership.ownerStableId == 0)) return false;
    item->ownership = ownership;
    if (ownership.owned() && item->identityTier < SurfaceItemIdentityTier::NamedOwned)
        item->identityTier = SurfaceItemIdentityTier::NamedOwned;
    return true;
}

bool SurfaceItemLedger::markArtifact(std::uint64_t itemStableId, std::uint64_t artifactStableId) {
    auto* item = findItem(itemStableId);
    if (!item || artifactStableId == 0) return false;
    item->artifactStableId = artifactStableId;
    item->identityTier = SurfaceItemIdentityTier::Artifact;
    item->provenance.preserve = true;
    return true;
}

void SurfaceItemLedger::invalidateReservationsForResource(std::uint64_t resourceStableId,
                                                          SurfaceReservationInvalidationReason reason,
                                                          std::uint64_t exceptReservationStableId) {
    std::vector<std::uint64_t> remove;
    for (const auto& reservation : reservations_) {
        if (reservation.resourceStableId != resourceStableId || reservation.stableId == exceptReservationStableId) continue;
        invalidations_.push_back({reservation.stableId, reservation.ownerStableId, reservation.dependentStableId,
                                  reservation.resourceStableId, reason});
        blockHaulsForReservation(reservation.stableId);
        remove.push_back(reservation.stableId);
    }
    if (!remove.empty()) {
        reservations_.erase(std::remove_if(reservations_.begin(), reservations_.end(), [&](const auto& reservation) {
            return std::binary_search(remove.begin(), remove.end(), reservation.stableId);
        }), reservations_.end());
    }
}

bool SurfaceItemLedger::moveItem(std::uint64_t itemStableId,
                                 SurfaceItemLocation destination,
                                 std::uint64_t authorizingReservationStableId) {
    auto* item = findItem(itemStableId);
    if (!item) return false;
    if (destination.kind == SurfaceItemLocationKind::Stockpile) {
        auto* stockpile = findStockpile(destination.holderStableId);
        if (!stockpile || !stockpileAccepts(*stockpile, item->key, item->ownership) ||
            !stockpileCanAcceptUnits(*stockpile, item->quantity)) return false;
        destination.cell = stockpile->cells.front();
    }
    invalidateReservationsForResource(itemStableId, SurfaceReservationInvalidationReason::ResourceMoved,
                                      authorizingReservationStableId);
    item = findItem(itemStableId);
    if (!item) return false;
    item->location = destination;
    return true;
}

bool SurfaceItemLedger::destroyItem(std::uint64_t itemStableId) {
    if (!findItem(itemStableId)) return false;
    invalidateReservationsForResource(itemStableId, SurfaceReservationInvalidationReason::ResourceDestroyed);
    for (auto& container : containers_) {
        container.cargoItemStableIds.erase(std::remove(container.cargoItemStableIds.begin(), container.cargoItemStableIds.end(), itemStableId),
                                           container.cargoItemStableIds.end());
    }
    for (auto& task : haulTasks_) {
        if (task.itemStableId == itemStableId && task.state != SurfaceHaulState::Completed && task.state != SurfaceHaulState::Cancelled)
            setHaulFailure(task, SurfaceHaulFailure::ResourceInvalidated);
    }
    items_.erase(std::remove_if(items_.begin(), items_.end(), [itemStableId](const auto& item) { return item.stableId == itemStableId; }), items_.end());
    return true;
}

std::uint64_t SurfaceItemLedger::reserveItem(std::uint64_t resourceStableId,
                                             std::uint64_t ownerStableId,
                                             SurfaceReservationKind kind,
                                             int quantity,
                                             std::uint64_t expiresTick,
                                             std::uint64_t dependentStableId) {
    const auto* item = findItem(resourceStableId);
    if (!item || ownerStableId == 0 || quantity <= 0 || quantity > item->quantity) return 0;
    if (const auto* existing = reservationForResource(resourceStableId)) {
        if (existing->ownerStableId == ownerStableId && existing->kind == kind && existing->quantity == quantity) return existing->stableId;
        return 0;
    }
    SurfaceReservationRecord reservation{};
    reservation.stableId = allocateStableId();
    reservation.kind = kind;
    reservation.ownerStableId = ownerStableId;
    reservation.dependentStableId = dependentStableId;
    reservation.resourceStableId = resourceStableId;
    reservation.quantity = quantity;
    reservation.expiresTick = expiresTick;
    reservations_.push_back(reservation);
    std::sort(reservations_.begin(), reservations_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return reservation.stableId;
}

void SurfaceItemLedger::blockHaulsForReservation(std::uint64_t reservationStableId,
                                                   SurfaceHaulFailure failure) {
    if (reservationStableId == 0) return;
    for (auto& task : haulTasks_) {
        if (task.reservationStableId != reservationStableId) continue;
        if (task.state == SurfaceHaulState::Completed || task.state == SurfaceHaulState::Cancelled) continue;
        setHaulFailure(task, failure);
    }
}

bool SurfaceItemLedger::releaseReservation(std::uint64_t reservationStableId,
                                           SurfaceReservationInvalidationReason reason,
                                           bool emitDiagnostic) {
    const auto it = std::lower_bound(reservations_.begin(), reservations_.end(), reservationStableId,
                                     [](const auto& reservation, std::uint64_t id) { return reservation.stableId < id; });
    if (it == reservations_.end() || it->stableId != reservationStableId) return false;
    if (emitDiagnostic) {
        invalidations_.push_back({it->stableId, it->ownerStableId, it->dependentStableId, it->resourceStableId, reason});
        if (reason != SurfaceReservationInvalidationReason::Released)
            blockHaulsForReservation(it->stableId);
    }
    reservations_.erase(it);
    return true;
}

void SurfaceItemLedger::expireReservations(std::uint64_t currentTick) {
    std::vector<std::uint64_t> expired;
    for (const auto& reservation : reservations_) {
        if (reservation.expiresTick != 0 && reservation.expiresTick <= currentTick) expired.push_back(reservation.stableId);
    }
    for (const auto id : expired) releaseReservation(id, SurfaceReservationInvalidationReason::LeaseExpired, true);
}

const SurfaceReservationRecord* SurfaceItemLedger::findReservation(std::uint64_t stableId) const {
    const auto it = std::lower_bound(reservations_.begin(), reservations_.end(), stableId,
                                     [](const auto& reservation, std::uint64_t id) { return reservation.stableId < id; });
    return it != reservations_.end() && it->stableId == stableId ? &*it : nullptr;
}

const SurfaceReservationRecord* SurfaceItemLedger::reservationForResource(std::uint64_t resourceStableId) const {
    const SurfaceReservationRecord* best = nullptr;
    for (const auto& reservation : reservations_) {
        if (reservation.resourceStableId == resourceStableId && (!best || reservation.stableId < best->stableId)) best = &reservation;
    }
    return best;
}

std::vector<SurfaceReservationInvalidation> SurfaceItemLedger::consumeReservationInvalidations() {
    auto result = std::move(invalidations_);
    invalidations_.clear();
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.reservationStableId < b.reservationStableId; });
    return result;
}

std::uint64_t SurfaceItemLedger::createHaulContainer(std::uint64_t ownerStableId, int capacityUnits) {
    if (ownerStableId == 0 || capacityUnits <= 0) return 0;
    SurfaceHaulContainer container{};
    container.stableId = allocateStableId();
    container.ownerStableId = ownerStableId;
    container.capacityUnits = capacityUnits;
    containers_.push_back(container);
    std::sort(containers_.begin(), containers_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return container.stableId;
}

const SurfaceHaulContainer* SurfaceItemLedger::findHaulContainer(std::uint64_t stableId) const {
    const auto it = std::lower_bound(containers_.begin(), containers_.end(), stableId,
                                     [](const auto& container, std::uint64_t id) { return container.stableId < id; });
    return it != containers_.end() && it->stableId == stableId ? &*it : nullptr;
}

int SurfaceItemLedger::containerUnits(const SurfaceHaulContainer& container,
                                      const std::vector<SurfaceItemRecord>& items) {
    int units = 0;
    for (const auto id : container.cargoItemStableIds) {
        const auto it = std::lower_bound(items.begin(), items.end(), id,
                                         [](const auto& item, std::uint64_t stableId) { return item.stableId < stableId; });
        if (it != items.end() && it->stableId == id) units += it->quantity;
    }
    return units;
}

void SurfaceItemLedger::setHaulFailure(SurfaceHaulTask& task, SurfaceHaulFailure failure) {
    task.state = SurfaceHaulState::Blocked;
    task.failure = failure;
}

std::uint64_t SurfaceItemLedger::beginHaul(std::uint64_t workerStableId,
                                           std::uint64_t sourceStockpileStableId,
                                           std::uint64_t destinationStockpileStableId,
                                           const SurfaceItemKey& key,
                                           int quantity,
                                           SurfaceHaulMode mode,
                                           std::uint64_t currentTick,
                                           std::uint64_t leaseTicks,
                                           std::uint64_t carrierStableId,
                                           std::uint64_t dependentStableId) {
    if (workerStableId == 0 || quantity <= 0 || (mode != SurfaceHaulMode::Hand && mode != SurfaceHaulMode::CartTrolley)) return 0;
    auto* source = findStockpile(sourceStockpileStableId);
    const auto resolvedDestination = resolveDestination(destinationStockpileStableId, key, {}, quantity, sourceStockpileStableId);
    auto* destination = findStockpile(resolvedDestination);
    if (!source || !destination) return 0;
    destinationStockpileStableId = resolvedDestination;
    if (bulkCount(sourceStockpileStableId, key) < quantity) return 0;

    if (mode == SurfaceHaulMode::CartTrolley) {
        const auto* container = findHaulContainer(carrierStableId);
        if (!container || !source->allowVehicles || !destination->allowVehicles ||
            containerUnits(*container, items_) > container->capacityUnits - quantity) return 0;
    } else {
        carrierStableId = workerStableId;
    }

    const auto itemId = promoteFromBulk(sourceStockpileStableId, key, quantity, SurfaceItemIdentityTier::Stack);
    if (!itemId) return 0;
    const auto reservationId = reserveItem(itemId, workerStableId, SurfaceReservationKind::Hauling, quantity,
                                           leaseTicks == 0 ? 0 : currentTick + leaseTicks, dependentStableId);
    if (!reservationId) {
        tryReaggregate(itemId, sourceStockpileStableId);
        return 0;
    }

    SurfaceItemLocation transit{};
    transit.kind = mode == SurfaceHaulMode::Hand ? SurfaceItemLocationKind::Carried : SurfaceItemLocationKind::Container;
    transit.holderStableId = carrierStableId;
    transit.cell = source->cells.front();
    if (!moveItem(itemId, transit, reservationId)) {
        releaseReservation(reservationId);
        auto* item = findItem(itemId);
        if (item) item->location = {SurfaceItemLocationKind::Stockpile, source->cells.front(), sourceStockpileStableId};
        tryReaggregate(itemId, sourceStockpileStableId);
        return 0;
    }
    if (mode == SurfaceHaulMode::CartTrolley) {
        auto it = std::lower_bound(containers_.begin(), containers_.end(), carrierStableId,
                                   [](const auto& container, std::uint64_t id) { return container.stableId < id; });
        it->cargoItemStableIds.push_back(itemId);
        std::sort(it->cargoItemStableIds.begin(), it->cargoItemStableIds.end());
    }

    SurfaceHaulTask task{};
    task.stableId = allocateStableId();
    task.workerStableId = workerStableId;
    task.mode = mode;
    task.state = SurfaceHaulState::InTransit;
    task.sourceStockpileStableId = sourceStockpileStableId;
    task.destinationStockpileStableId = destinationStockpileStableId;
    task.carrierStableId = carrierStableId;
    task.itemStableId = itemId;
    task.reservationStableId = reservationId;
    haulTasks_.push_back(task);
    std::sort(haulTasks_.begin(), haulTasks_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    return task.stableId;
}

SurfaceHaulTask* SurfaceItemLedger::findHaulTask(std::uint64_t stableId) {
    const auto it = std::lower_bound(haulTasks_.begin(), haulTasks_.end(), stableId,
                                     [](const auto& task, std::uint64_t id) { return task.stableId < id; });
    return it != haulTasks_.end() && it->stableId == stableId ? &*it : nullptr;
}

const SurfaceHaulTask* SurfaceItemLedger::findHaulTask(std::uint64_t stableId) const {
    const auto it = std::lower_bound(haulTasks_.begin(), haulTasks_.end(), stableId,
                                     [](const auto& task, std::uint64_t id) { return task.stableId < id; });
    return it != haulTasks_.end() && it->stableId == stableId ? &*it : nullptr;
}

bool SurfaceItemLedger::completeHaul(std::uint64_t haulTaskStableId) {
    auto* task = findHaulTask(haulTaskStableId);
    if (!task || task->state != SurfaceHaulState::InTransit) return false;
    auto* item = findItem(task->itemStableId);
    auto* destination = findStockpile(task->destinationStockpileStableId);
    const auto* reservation = findReservation(task->reservationStableId);
    if (!item || !destination || !reservation || reservation->resourceStableId != task->itemStableId) {
        setHaulFailure(*task, SurfaceHaulFailure::ResourceInvalidated);
        return false;
    }
    if (!stockpileAccepts(*destination, item->key, item->ownership)) {
        setHaulFailure(*task, SurfaceHaulFailure::NoAcceptedItem);
        return false;
    }
    if (!stockpileCanAcceptUnits(*destination, item->quantity)) {
        setHaulFailure(*task, SurfaceHaulFailure::DestinationFull);
        return false;
    }
    if (task->mode == SurfaceHaulMode::CartTrolley) {
        auto it = std::lower_bound(containers_.begin(), containers_.end(), task->carrierStableId,
                                   [](const auto& container, std::uint64_t id) { return container.stableId < id; });
        if (it == containers_.end() || it->stableId != task->carrierStableId) {
            setHaulFailure(*task, SurfaceHaulFailure::ContainerMissing);
            return false;
        }
        it->cargoItemStableIds.erase(std::remove(it->cargoItemStableIds.begin(), it->cargoItemStableIds.end(), task->itemStableId),
                                     it->cargoItemStableIds.end());
    }
    const SurfaceItemLocation destinationLocation{SurfaceItemLocationKind::Stockpile, destination->cells.front(), destination->stableId};
    if (!moveItem(task->itemStableId, destinationLocation, task->reservationStableId)) {
        setHaulFailure(*task, SurfaceHaulFailure::DestinationFull);
        return false;
    }
    releaseReservation(task->reservationStableId);
    task = findHaulTask(haulTaskStableId);
    if (!task) return false;
    task->state = SurfaceHaulState::Completed;
    task->failure = SurfaceHaulFailure::None;
    tryReaggregate(task->itemStableId, task->destinationStockpileStableId);
    return true;
}

bool SurfaceItemLedger::cancelHaul(std::uint64_t haulTaskStableId) {
    auto* task = findHaulTask(haulTaskStableId);
    if (!task || task->state == SurfaceHaulState::Completed || task->state == SurfaceHaulState::Cancelled) return false;
    auto* item = findItem(task->itemStableId);
    auto* source = findStockpile(task->sourceStockpileStableId);
    if (task->mode == SurfaceHaulMode::CartTrolley) {
        auto it = std::lower_bound(containers_.begin(), containers_.end(), task->carrierStableId,
                                   [](const auto& container, std::uint64_t id) { return container.stableId < id; });
        if (it != containers_.end() && it->stableId == task->carrierStableId)
            it->cargoItemStableIds.erase(std::remove(it->cargoItemStableIds.begin(), it->cargoItemStableIds.end(), task->itemStableId), it->cargoItemStableIds.end());
    }
    if (item && source && stockpileAccepts(*source, item->key, item->ownership) && stockpileCanAcceptUnits(*source, item->quantity)) {
        item->location = {SurfaceItemLocationKind::Stockpile, source->cells.front(), source->stableId};
    }
    releaseReservation(task->reservationStableId, SurfaceReservationInvalidationReason::OwnerCancelled, true);
    task = findHaulTask(haulTaskStableId);
    if (!task) return false;
    task->state = SurfaceHaulState::Cancelled;
    task->failure = SurfaceHaulFailure::None;
    if (item && source) tryReaggregate(item->stableId, source->stableId);
    return true;
}

std::optional<SurfaceExternalHaulRequest> SurfaceItemLedger::prepareExternalHaul(
    std::uint64_t workerOrControllerStableId,
    std::uint64_t transportStableId,
    std::uint64_t sourceStockpileStableId,
    std::uint64_t destinationStockpileStableId,
    const SurfaceItemKey& key,
    int quantity,
    SurfaceHaulMode mode,
    std::uint64_t currentTick,
    std::uint64_t leaseTicks,
    std::uint64_t dependentStableId) {
    if (mode != SurfaceHaulMode::Conveyor && mode != SurfaceHaulMode::Drone &&
        mode != SurfaceHaulMode::CargoRail && mode != SurfaceHaulMode::Ship) return std::nullopt;
    if (workerOrControllerStableId == 0 || transportStableId == 0 || quantity <= 0) return std::nullopt;
    auto* source = findStockpile(sourceStockpileStableId);
    const auto resolvedDestination = resolveDestination(destinationStockpileStableId, key, {}, quantity, sourceStockpileStableId);
    auto* destination = findStockpile(resolvedDestination);
    if (!source || !destination || bulkCount(sourceStockpileStableId, key) < quantity) return std::nullopt;
    destinationStockpileStableId = resolvedDestination;
    if ((mode == SurfaceHaulMode::Drone || mode == SurfaceHaulMode::CargoRail || mode == SurfaceHaulMode::Ship) &&
        (!source->allowVehicles || !destination->allowVehicles)) return std::nullopt;

    const auto itemId = promoteFromBulk(sourceStockpileStableId, key, quantity, SurfaceItemIdentityTier::Stack);
    if (!itemId) return std::nullopt;
    const auto reservationId = reserveItem(itemId, workerOrControllerStableId, SurfaceReservationKind::Hauling, quantity,
                                           leaseTicks == 0 ? 0 : currentTick + leaseTicks, dependentStableId);
    if (!reservationId) {
        tryReaggregate(itemId, sourceStockpileStableId);
        return std::nullopt;
    }
    SurfaceItemLocation transit{};
    transit.kind = mode == SurfaceHaulMode::Ship ? SurfaceItemLocationKind::TradeManifest : SurfaceItemLocationKind::Container;
    transit.holderStableId = transportStableId;
    transit.cell = source->cells.front();
    if (!moveItem(itemId, transit, reservationId)) {
        releaseReservation(reservationId);
        auto* item = findItem(itemId);
        if (item) item->location = {SurfaceItemLocationKind::Stockpile, source->cells.front(), source->stableId};
        tryReaggregate(itemId, source->stableId);
        return std::nullopt;
    }
    return SurfaceExternalHaulRequest{mode, allocateStableId(), itemId, reservationId, sourceStockpileStableId,
                                      destinationStockpileStableId, transportStableId};
}

bool SurfaceItemLedger::commitExternalDelivery(const SurfaceExternalHaulRequest& request) {
    if (request.requestStableId == 0 || request.itemStableId == 0 || request.reservationStableId == 0) return false;
    const auto* reservation = findReservation(request.reservationStableId);
    auto* item = findItem(request.itemStableId);
    auto* destination = findStockpile(request.destinationStockpileStableId);
    if (!reservation || !item || !destination || reservation->resourceStableId != request.itemStableId ||
        !stockpileAccepts(*destination, item->key, item->ownership) || !stockpileCanAcceptUnits(*destination, item->quantity)) return false;
    if (!moveItem(item->stableId, {SurfaceItemLocationKind::Stockpile, destination->cells.front(), destination->stableId}, request.reservationStableId)) return false;
    releaseReservation(request.reservationStableId);
    tryReaggregate(request.itemStableId, request.destinationStockpileStableId);
    return true;
}

int SurfaceItemLedger::totalQuantity(int itemId) const {
    long long total = 0;
    for (const auto& stockpile : stockpiles_) for (const auto& bucket : stockpile.bulk) if (bucket.key.itemId == itemId) total += bucket.quantity;
    for (const auto& item : items_) if (item.key.itemId == itemId) total += item.quantity;
    return static_cast<int>(std::min<long long>(total, std::numeric_limits<int>::max()));
}

SurfaceItemLedgerSnapshot SurfaceItemLedger::snapshot() const {
    return {1, worldSeed_, nextSerial_, items_, stockpiles_, reservations_, containers_, haulTasks_};
}

bool SurfaceItemLedger::restore(const SurfaceItemLedgerSnapshot& snapshotValue, std::string* error) {
    if (snapshotValue.schemaVersion != 1) { setError(error, "unsupported item-ledger schema"); return false; }
    if (snapshotValue.worldSeed != worldSeed_) { setError(error, "item-ledger world seed mismatch"); return false; }
    SurfaceItemLedger candidate(worldSeed_);
    candidate.nextSerial_ = std::max<std::uint64_t>(1, snapshotValue.nextSerial);

    std::unordered_set<std::uint64_t> ids;
    auto claimId = [&](std::uint64_t id) {
        return id != 0 && ids.insert(id).second;
    };

    candidate.stockpiles_ = snapshotValue.stockpiles;
    for (auto& stockpile : candidate.stockpiles_) {
        if (!claimId(stockpile.stableId) || stockpile.cells.empty() || stockpile.maxUnits <= 0 ||
            stockpile.filter.minimumQuality > stockpile.filter.maximumQuality ||
            stockpile.filter.minimumConditionPermille > 1000) {
            setError(error, "invalid stockpile record"); return false;
        }
        std::sort(stockpile.cells.begin(), stockpile.cells.end(), addressLess);
        stockpile.cells.erase(std::unique(stockpile.cells.begin(), stockpile.cells.end()), stockpile.cells.end());
        normalizeIds(stockpile.giveToLinks);
        normalizeIds(stockpile.takeFromLinks);
        normalizeBulk(stockpile);
    }
    std::sort(candidate.stockpiles_.begin(), candidate.stockpiles_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });

    for (const auto& stockpile : candidate.stockpiles_) {
        for (const auto id : stockpile.giveToLinks) if (!candidate.findStockpile(id)) { setError(error, "stockpile give link unresolved"); return false; }
        for (const auto id : stockpile.takeFromLinks) if (!candidate.findStockpile(id)) { setError(error, "stockpile take link unresolved"); return false; }
        if (stockpile.overflowStockpileStableId && !candidate.findStockpile(stockpile.overflowStockpileStableId)) { setError(error, "stockpile overflow unresolved"); return false; }
    }

    candidate.items_ = snapshotValue.items;
    for (const auto& item : candidate.items_) {
        if (!claimId(item.stableId) || item.quantity <= 0 || !validKey(item.key) || item.identityTier == SurfaceItemIdentityTier::BulkAggregate) {
            setError(error, "invalid item record"); return false;
        }
        if ((item.ownership.kind == SurfaceOwnerKind::None) != (item.ownership.ownerStableId == 0)) { setError(error, "invalid ownership claim"); return false; }
        if (item.location.kind == SurfaceItemLocationKind::Stockpile && !candidate.findStockpile(item.location.holderStableId)) {
            setError(error, "item stockpile location unresolved"); return false;
        }
    }
    std::sort(candidate.items_.begin(), candidate.items_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    for (const auto& stockpile : candidate.stockpiles_) {
        if (candidate.stockpileUnits(stockpile.stableId) > stockpile.maxUnits) {
            setError(error, "stockpile exceeds capacity after explicit item restore"); return false;
        }
    }

    candidate.containers_ = snapshotValue.containers;
    for (auto& container : candidate.containers_) {
        if (!claimId(container.stableId) || container.ownerStableId == 0 || container.capacityUnits <= 0) { setError(error, "invalid haul container"); return false; }
        normalizeIds(container.cargoItemStableIds);
        for (const auto id : container.cargoItemStableIds) if (!candidate.findItem(id)) { setError(error, "container cargo unresolved"); return false; }
        if (containerUnits(container, candidate.items_) > container.capacityUnits) { setError(error, "haul container over capacity"); return false; }
    }
    std::sort(candidate.containers_.begin(), candidate.containers_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });

    candidate.reservations_ = snapshotValue.reservations;
    std::unordered_set<std::uint64_t> reservedResources;
    for (const auto& reservation : candidate.reservations_) {
        const auto* item = candidate.findItem(reservation.resourceStableId);
        if (!claimId(reservation.stableId) || reservation.ownerStableId == 0 || !item || reservation.quantity <= 0 || reservation.quantity > item->quantity ||
            !reservedResources.insert(reservation.resourceStableId).second) { setError(error, "invalid reservation record"); return false; }
    }
    std::sort(candidate.reservations_.begin(), candidate.reservations_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });

    candidate.haulTasks_ = snapshotValue.haulTasks;
    for (const auto& task : candidate.haulTasks_) {
        if (!claimId(task.stableId) || task.workerStableId == 0 || !candidate.findStockpile(task.sourceStockpileStableId) ||
            !candidate.findStockpile(task.destinationStockpileStableId)) { setError(error, "invalid haul task"); return false; }
        if ((task.state == SurfaceHaulState::Reserved || task.state == SurfaceHaulState::InTransit) &&
            (!candidate.findItem(task.itemStableId) || !candidate.findReservation(task.reservationStableId))) {
            setError(error, "active haul dependency unresolved"); return false;
        }
    }
    std::sort(candidate.haulTasks_.begin(), candidate.haulTasks_.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });

    for (const auto& stockpile : candidate.stockpiles_) {
        if (candidate.stockpileUnits(stockpile.stableId) > stockpile.maxUnits) { setError(error, "stockpile over capacity"); return false; }
    }

    *this = std::move(candidate);
    if (error) error->clear();
    return true;
}

const char* SurfaceItemLedger::haulFailureName(SurfaceHaulFailure failure) {
    switch (failure) {
        case SurfaceHaulFailure::None: return "none";
        case SurfaceHaulFailure::SourceMissing: return "source missing";
        case SurfaceHaulFailure::DestinationMissing: return "destination missing";
        case SurfaceHaulFailure::NoAcceptedItem: return "destination filter rejected item";
        case SurfaceHaulFailure::DestinationFull: return "destination full";
        case SurfaceHaulFailure::ReservationConflict: return "reservation conflict";
        case SurfaceHaulFailure::ContainerMissing: return "container missing";
        case SurfaceHaulFailure::ContainerFull: return "container full";
        case SurfaceHaulFailure::UnsupportedTransport: return "unsupported transport";
        case SurfaceHaulFailure::ResourceInvalidated: return "reserved resource invalidated";
        case SurfaceHaulFailure::LinkPolicyRejected: return "stockpile link policy rejected route";
    }
    return "unknown";
}

const char* SurfaceItemLedger::reservationInvalidationName(SurfaceReservationInvalidationReason reason) {
    switch (reason) {
        case SurfaceReservationInvalidationReason::Released: return "released";
        case SurfaceReservationInvalidationReason::ResourceMoved: return "resource moved";
        case SurfaceReservationInvalidationReason::ResourceDestroyed: return "resource destroyed";
        case SurfaceReservationInvalidationReason::ResourceChanged: return "resource changed";
        case SurfaceReservationInvalidationReason::LeaseExpired: return "lease expired";
        case SurfaceReservationInvalidationReason::OwnerCancelled: return "owner cancelled";
    }
    return "unknown";
}

} // namespace elysium
