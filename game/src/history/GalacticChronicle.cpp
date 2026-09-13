// Intended function: imported history implementation for GalacticChronicle; preserves the agent-authored subsystem contract for later integration/debugging.
#include "history/GalacticChronicle.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr std::uint64_t kEventIdLabel = 0x4849535445564E54ULL; // HISTEVNT
constexpr std::uint64_t kCivilizationLabel = 0x4849535443495649ULL; // HISTCIVI
constexpr std::uint64_t kSiteLabel = 0x4849535453495445ULL; // HISTSITE
constexpr std::uint64_t kOrganizationLabel = 0x484953544F52474EULL; // HISTORGN
constexpr std::uint64_t kEpochLabel = 0x4849535445504F43ULL; // HISTEPOC
constexpr std::uint64_t kFigureLabel = 0x4849535446494752ULL; // HISTFIGR

std::uint64_t stableStringHash(std::string_view text) {
    std::uint64_t h = 1469598103934665603ULL;
    for (const unsigned char c : text) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return mix64(h);
}

std::string lowerAscii(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (const unsigned char c : text) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

bool containsCaseInsensitive(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) return true;
    const auto h = lowerAscii(haystack);
    const auto n = lowerAscii(needle);
    return h.find(n) != std::string::npos;
}

template <class T>
void sortUnique(std::vector<T>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

void canonicalizeDraft(HistoricalEventDraft& draft) {
    std::sort(draft.participants.begin(), draft.participants.end(), [](const auto& a, const auto& b) {
        return std::tie(a.stableId, a.role) < std::tie(b.stableId, b.role);
    });
    draft.participants.erase(std::unique(draft.participants.begin(), draft.participants.end()), draft.participants.end());
    sortUnique(draft.artifacts);
    sortUnique(draft.organizations);
    sortUnique(draft.causeRefs);
    std::sort(draft.payload.begin(), draft.payload.end(), [](const auto& a, const auto& b) {
        return std::tie(a.key, a.value) < std::tie(b.key, b.value);
    });
    draft.payload.erase(std::unique(draft.payload.begin(), draft.payload.end()), draft.payload.end());
}

std::uint64_t derivedId(std::uint64_t seed, std::uint64_t label, std::uint64_t a, std::uint64_t b = 0) {
    auto h = mix64(seed ^ label);
    h = mix64(h ^ a);
    h = mix64(h ^ b);
    return h == 0 ? 1 : h;
}

std::string generatedName(std::uint64_t seed, std::string_view prefix, std::uint32_t ordinal) {
    static constexpr const char* a[] = {"Aster", "Vara", "Keth", "Orin", "Nym", "Sol", "Dru", "Eid"};
    static constexpr const char* b[] = {"Reach", "Compact", "Concord", "Enclave", "Archive", "League", "Covenant", "March"};
    const auto h = mix64(seed ^ stableStringHash(prefix) ^ ordinal);
    std::ostringstream out;
    out << prefix << ' ' << a[(h >> 8U) & 7U] << ' ' << b[(h >> 20U) & 7U] << ' ' << (ordinal + 1U);
    return out.str();
}

bool validEventType(int value) {
    return value >= 0 && value <= static_cast<int>(HistoryEventType::PlayerMilestone);
}

bool validOrganizationKind(int value) {
    return value >= 0 && value <= static_cast<int>(OrganizationKind::SmugglerNetwork);
}

bool validSiteStatus(int value) {
    return value >= 0 && value <= static_cast<int>(SiteStatus::Reclaimed);
}

std::vector<std::size_t> intersectIndexes(const std::vector<std::size_t>& a, const std::vector<std::size_t>& b) {
    std::vector<std::size_t> out;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(out));
    return out;
}

std::optional<std::uint64_t> payloadUint64(const HistoricalEventDraft& event, std::string_view key) {
    for (const auto& field : event.payload) {
        if (field.key != key) continue;
        try {
            std::size_t consumed = 0;
            const auto value = std::stoull(field.value, &consumed, 10);
            if (consumed == field.value.size()) return value;
        } catch (...) {
        }
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<DiplomaticStance> stanceFromEvent(HistoryEventType type) {
    switch (type) {
        case HistoryEventType::WarDeclared:
        case HistoryEventType::Battle:
        case HistoryEventType::Raid:
        case HistoryEventType::Siege:
        case HistoryEventType::Occupation:
            return DiplomaticStance::War;
        case HistoryEventType::Embargo:
            return DiplomaticStance::Embargo;
        case HistoryEventType::Treaty:
            return DiplomaticStance::Treaty;
        case HistoryEventType::Armistice:
        case HistoryEventType::Liberation:
        case HistoryEventType::Amnesty:
            return DiplomaticStance::Peace;
        default:
            return std::nullopt;
    }
}

} // namespace

const char* historyEventTypeName(HistoryEventType type) {
    switch (type) {
        case HistoryEventType::Birth: return "Birth";
        case HistoryEventType::Naming: return "Naming";
        case HistoryEventType::ComingOfAge: return "ComingOfAge";
        case HistoryEventType::Death: return "Death";
        case HistoryEventType::Disappearance: return "Disappearance";
        case HistoryEventType::Transformation: return "Transformation";
        case HistoryEventType::RelationshipFormed: return "RelationshipFormed";
        case HistoryEventType::RelationshipDissolved: return "RelationshipDissolved";
        case HistoryEventType::Mentorship: return "Mentorship";
        case HistoryEventType::Rivalry: return "Rivalry";
        case HistoryEventType::Oath: return "Oath";
        case HistoryEventType::Betrayal: return "Betrayal";
        case HistoryEventType::SiteFounded: return "SiteFounded";
        case HistoryEventType::SiteExpanded: return "SiteExpanded";
        case HistoryEventType::SiteRenamed: return "SiteRenamed";
        case HistoryEventType::SiteClaimed: return "SiteClaimed";
        case HistoryEventType::SiteAbandoned: return "SiteAbandoned";
        case HistoryEventType::SiteDestroyed: return "SiteDestroyed";
        case HistoryEventType::SiteContaminated: return "SiteContaminated";
        case HistoryEventType::SiteReclaimed: return "SiteReclaimed";
        case HistoryEventType::OrganizationFounded: return "OrganizationFounded";
        case HistoryEventType::OrganizationDissolved: return "OrganizationDissolved";
        case HistoryEventType::OfficeAssumed: return "OfficeAssumed";
        case HistoryEventType::OfficeRemoved: return "OfficeRemoved";
        case HistoryEventType::Schism: return "Schism";
        case HistoryEventType::WarDeclared: return "WarDeclared";
        case HistoryEventType::Battle: return "Battle";
        case HistoryEventType::Raid: return "Raid";
        case HistoryEventType::Siege: return "Siege";
        case HistoryEventType::Occupation: return "Occupation";
        case HistoryEventType::Liberation: return "Liberation";
        case HistoryEventType::Armistice: return "Armistice";
        case HistoryEventType::Treaty: return "Treaty";
        case HistoryEventType::Embargo: return "Embargo";
        case HistoryEventType::Migration: return "Migration";
        case HistoryEventType::ArtifactCreated: return "ArtifactCreated";
        case HistoryEventType::ArtifactStolen: return "ArtifactStolen";
        case HistoryEventType::ArtifactGifted: return "ArtifactGifted";
        case HistoryEventType::ArtifactInherited: return "ArtifactInherited";
        case HistoryEventType::ArtifactLost: return "ArtifactLost";
        case HistoryEventType::ArtifactRecovered: return "ArtifactRecovered";
        case HistoryEventType::ArtifactDestroyed: return "ArtifactDestroyed";
        case HistoryEventType::Discovery: return "Discovery";
        case HistoryEventType::Filing: return "Filing";
        case HistoryEventType::ScientificBreakthrough: return "ScientificBreakthrough";
        case HistoryEventType::ForbiddenProtocolLearned: return "ForbiddenProtocolLearned";
        case HistoryEventType::ExpeditionResult: return "ExpeditionResult";
        case HistoryEventType::Crime: return "Crime";
        case HistoryEventType::Accusation: return "Accusation";
        case HistoryEventType::Investigation: return "Investigation";
        case HistoryEventType::Conviction: return "Conviction";
        case HistoryEventType::Exile: return "Exile";
        case HistoryEventType::Pardon: return "Pardon";
        case HistoryEventType::MegathreatAppeared: return "MegathreatAppeared";
        case HistoryEventType::MegathreatRampage: return "MegathreatRampage";
        case HistoryEventType::MegathreatDefeated: return "MegathreatDefeated";
        case HistoryEventType::MegathreatImprisoned: return "MegathreatImprisoned";
        case HistoryEventType::PlanetaryDisaster: return "PlanetaryDisaster";
        case HistoryEventType::ReactorAccident: return "ReactorAccident";
        case HistoryEventType::SyndromeOutbreak: return "SyndromeOutbreak";
        case HistoryEventType::EcologicalCollapse: return "EcologicalCollapse";
        case HistoryEventType::ImperialInspection: return "ImperialInspection";
        case HistoryEventType::ImperialWarrant: return "ImperialWarrant";
        case HistoryEventType::RegisterAction: return "RegisterAction";
        case HistoryEventType::CourtRuling: return "CourtRuling";
        case HistoryEventType::Amnesty: return "Amnesty";
        case HistoryEventType::Annexation: return "Annexation";
        case HistoryEventType::FirstLanding: return "FirstLanding";
        case HistoryEventType::FirstClaim: return "FirstClaim";
        case HistoryEventType::FortressRetired: return "FortressRetired";
        case HistoryEventType::FortressReclaimed: return "FortressReclaimed";
        case HistoryEventType::DirectOperativeIntervention: return "DirectOperativeIntervention";
        case HistoryEventType::PlayerMilestone: return "PlayerMilestone";
    }
    return "Unknown";
}

const char* organizationKindName(OrganizationKind kind) {
    switch (kind) {
        case OrganizationKind::CivilizationAdministration: return "CivilizationAdministration";
        case OrganizationKind::LocalGovernment: return "LocalGovernment";
        case OrganizationKind::Guild: return "Guild";
        case OrganizationKind::TradeNetwork: return "TradeNetwork";
        case OrganizationKind::Faith: return "Faith";
        case OrganizationKind::MilitaryFormation: return "MilitaryFormation";
        case OrganizationKind::ResearchCircle: return "ResearchCircle";
        case OrganizationKind::SmugglerNetwork: return "SmugglerNetwork";
    }
    return "Organization";
}

std::uint32_t HistoricalSignificancePolicy::score(const HistoricalSignificanceInput& input) const {
    std::uint32_t value = 0;
    value += input.named ? 10U : 0U;
    value += input.holdsOffice ? 8U : 0U;
    value += input.ownsArtifact ? 10U : 0U;
    value += input.majorCombat ? 6U : 0U;
    value += input.relatedToHistoricalFigure ? 4U : 0U;
    value += input.unusualTransformation ? 10U : 0U;
    value += input.playerInteraction ? 8U : 0U;
    value += input.authorship ? 5U : 0U;
    value += input.discovery ? 6U : 0U;
    value += input.leadership ? 8U : 0U;
    value += input.majorEventParticipant ? 6U : 0U;
    return value;
}

bool HistoricalSignificancePolicy::shouldPromote(const HistoricalSignificanceInput& input) const {
    return score(input) >= promotionThreshold_;
}

GalacticChronicle::GalacticChronicle(std::uint64_t historySeed,
                                     std::uint32_t generatorVersion,
                                     std::uint64_t generatorFingerprint)
    : historySeed_(historySeed), generatorVersion_(generatorVersion), generatorFingerprint_(generatorFingerprint) {}

namespace {
template <class Map, class Record>
bool insertUniqueRecord(Map& map, Record record, std::uint64_t id, const char* noun, std::string* error) {
    if (id == 0) {
        if (error) *error = std::string(noun) + " ID must be nonzero";
        return false;
    }
    if (map.contains(id)) {
        if (error) *error = std::string("duplicate ") + noun + " ID";
        return false;
    }
    map.emplace(id, std::move(record));
    if (error) error->clear();
    return true;
}
} // namespace

bool GalacticChronicle::registerCivilization(CivilizationRecord record, std::string* error) {
    sortUnique(record.territorySites);
    sortUnique(record.organizations);
    return insertUniqueRecord(civilizations_, std::move(record), record.id, "civilization", error);
}

bool GalacticChronicle::registerSite(SiteRecord record, std::string* error) {
    return insertUniqueRecord(sites_, std::move(record), record.id, "site", error);
}

bool GalacticChronicle::registerOrganization(OrganizationRecord record, std::string* error) {
    return insertUniqueRecord(organizations_, std::move(record), record.id, "organization", error);
}

bool GalacticChronicle::registerFigure(HistoricalFigureRecord record, std::string* error) {
    return insertUniqueRecord(figures_, std::move(record), record.stableId, "figure", error);
}

bool GalacticChronicle::registerArtifact(ArtifactRecord record, std::string* error) {
    return insertUniqueRecord(artifacts_, std::move(record), record.id, "artifact", error);
}

const CivilizationRecord* GalacticChronicle::civilization(OrganizationId id) const {
    const auto it = civilizations_.find(id); return it == civilizations_.end() ? nullptr : &it->second;
}
const SiteRecord* GalacticChronicle::site(SiteId id) const {
    const auto it = sites_.find(id); return it == sites_.end() ? nullptr : &it->second;
}
const OrganizationRecord* GalacticChronicle::organization(OrganizationId id) const {
    const auto it = organizations_.find(id); return it == organizations_.end() ? nullptr : &it->second;
}
const HistoricalFigureRecord* GalacticChronicle::figure(HistoryStableId id) const {
    const auto it = figures_.find(id); return it == figures_.end() ? nullptr : &it->second;
}
const ArtifactRecord* GalacticChronicle::artifact(ArtifactId id) const {
    const auto it = artifacts_.find(id); return it == artifacts_.end() ? nullptr : &it->second;
}

bool GalacticChronicle::validateDraft(HistoricalEventDraft& draft, std::string* error) const {
    canonicalizeDraft(draft);
    if (draft.importance > 100) {
        if (error) *error = "historical event importance exceeds 100";
        return false;
    }
    for (const auto& participant : draft.participants) {
        if (participant.stableId == 0) {
            if (error) *error = "historical event participant ID must be nonzero";
            return false;
        }
    }
    for (const auto id : draft.artifacts) {
        if (id == 0) { if (error) *error = "historical event artifact ID must be nonzero"; return false; }
    }
    for (const auto id : draft.organizations) {
        if (id == 0) { if (error) *error = "historical event organization ID must be nonzero"; return false; }
    }
    for (const auto cause : draft.causeRefs) {
        if (cause == 0 || !eventById_.contains(cause)) {
            if (error) *error = "historical event cause reference is unresolved";
            return false;
        }
    }
    if (error) error->clear();
    return true;
}

HistoricalEventId GalacticChronicle::allocateEventId(const HistoricalEventDraft& draft) {
    for (;;) {
        auto h = mix64(historySeed_ ^ kEventIdLabel ^ nextEventSerial_++);
        h = mix64(h ^ draft.timestamp);
        h = mix64(h ^ static_cast<std::uint64_t>(draft.type));
        h = mix64(h ^ draft.location.siteId);
        if (h != 0 && !eventById_.contains(h)) return h;
    }
}

HistoricalEventId GalacticChronicle::append(HistoricalEventDraft draft, std::string* error) {
    if (!validateDraft(draft, error)) return 0;
    HistoricalEventRecord record{};
    static_cast<HistoricalEventDraft&>(record) = std::move(draft);
    record.eventId = allocateEventId(record);
    events_.push_back(std::move(record));
    const auto index = events_.size() - 1;
    indexEvent(index);
    applyEventToDurableSummaries(events_[index]);
    if (error) error->clear();
    return events_[index].eventId;
}

HistoricalEventId GalacticChronicle::appendPlayerMilestone(std::uint64_t timestamp,
                                                            HistoryEventType type,
                                                            HistoryStableId playerStableId,
                                                            SiteId siteId,
                                                            std::vector<HistoryPayloadField> payload,
                                                            std::string* error) {
    if (type != HistoryEventType::FirstLanding && type != HistoryEventType::FirstClaim &&
        type != HistoryEventType::FortressRetired && type != HistoryEventType::FortressReclaimed &&
        type != HistoryEventType::DirectOperativeIntervention && type != HistoryEventType::PlayerMilestone) {
        if (error) *error = "event type is not a player milestone type";
        return 0;
    }
    HistoricalEventDraft draft{};
    draft.timestamp = timestamp;
    draft.type = type;
    draft.location.siteId = siteId;
    if (const auto* siteRecord = site(siteId)) {
        draft.location.systemIndex = siteRecord->systemIndex;
        draft.location.planetIndex = siteRecord->planetIndex;
    }
    draft.participants.push_back({playerStableId, "player"});
    draft.payload = std::move(payload);
    draft.importance = 90;
    return append(std::move(draft), error);
}

HistoricalEventId GalacticChronicle::appendFortressRetired(std::uint64_t timestamp,
                                                             SiteId siteId,
                                                             HistoryStableId actorStableId,
                                                             std::string* error) {
    return appendPlayerMilestone(timestamp, HistoryEventType::FortressRetired, actorStableId, siteId, {}, error);
}

HistoricalEventId GalacticChronicle::appendFortressReclaimed(std::uint64_t timestamp,
                                                               SiteId siteId,
                                                               HistoryStableId actorStableId,
                                                               std::string* error) {
    return appendPlayerMilestone(timestamp, HistoryEventType::FortressReclaimed, actorStableId, siteId, {}, error);
}

HistoricalEventId GalacticChronicle::appendSiteOwnershipChange(std::uint64_t timestamp,
                                                                HistoryEventType type,
                                                                SiteId siteId,
                                                                OrganizationId newOwnerOrganizationId,
                                                                std::vector<HistoryParticipant> participants,
                                                                std::vector<HistoricalEventId> causeRefs,
                                                                std::string* error) {
    if (type != HistoryEventType::SiteClaimed && type != HistoryEventType::Occupation &&
        type != HistoryEventType::Liberation && type != HistoryEventType::Annexation &&
        type != HistoryEventType::SiteReclaimed) {
        if (error) *error = "event type is not a site ownership transition";
        return 0;
    }
    const auto* siteRecord = site(siteId);
    if (!siteRecord) {
        if (error) *error = "site ownership transition references unknown site";
        return 0;
    }
    if (newOwnerOrganizationId == 0 || (!organization(newOwnerOrganizationId) && !civilization(newOwnerOrganizationId))) {
        if (error) *error = "site ownership transition references unknown owner organization";
        return 0;
    }

    HistoricalEventDraft draft{};
    draft.timestamp = timestamp;
    draft.type = type;
    draft.location = {siteRecord->systemIndex, siteRecord->planetIndex, siteId};
    draft.participants = std::move(participants);
    draft.organizations = {siteRecord->ownerOrganizationId, newOwnerOrganizationId};
    draft.organizations.erase(std::remove(draft.organizations.begin(), draft.organizations.end(), 0), draft.organizations.end());
    draft.causeRefs = std::move(causeRefs);
    draft.payload = {{"previous_owner_org", std::to_string(siteRecord->ownerOrganizationId)},
                     {"new_owner_org", std::to_string(newOwnerOrganizationId)}};
    draft.importance = 85;
    return append(std::move(draft), error);
}

HistoricalEventId GalacticChronicle::appendOrganizationSchism(std::uint64_t timestamp,
                                                                OrganizationId parentOrganizationId,
                                                                OrganizationRecord child,
                                                                SiteId siteId,
                                                                std::string* error) {
    const auto* parent = organization(parentOrganizationId);
    if (!parent) {
        if (error) *error = "organization schism references unknown parent organization";
        return 0;
    }
    if (child.id == 0 || child.id == parentOrganizationId) {
        if (error) *error = "organization schism child ID must be nonzero and distinct";
        return 0;
    }
    if (child.civilizationId == 0) child.civilizationId = parent->civilizationId;
    if (child.homeSiteId == 0) child.homeSiteId = siteId != 0 ? siteId : parent->homeSiteId;
    const auto childId = child.id;
    if (!registerOrganization(std::move(child), error)) return 0;

    HistoricalEventDraft draft{};
    draft.timestamp = timestamp;
    draft.type = HistoryEventType::Schism;
    draft.organizations = {parentOrganizationId, childId};
    draft.payload = {{"parent_organization_id", std::to_string(parentOrganizationId)},
                     {"child_organization_id", std::to_string(childId)}};
    draft.importance = 70;
    if (const auto* s = site(siteId != 0 ? siteId : parent->homeSiteId))
        draft.location = {s->systemIndex, s->planetIndex, s->id};
    const auto eventId = append(std::move(draft), error);
    if (eventId == 0) {
        organizations_.erase(childId);
        return 0;
    }
    return eventId;
}

const HistoricalEventRecord* GalacticChronicle::event(HistoricalEventId id) const {
    const auto it = eventById_.find(id);
    return it == eventById_.end() ? nullptr : &events_[it->second];
}

void GalacticChronicle::indexEvent(std::size_t index) {
    const auto& record = events_[index];
    eventById_[record.eventId] = index;
    typeIndex_[record.type].push_back(index);
    if (record.location.siteId != 0) siteIndex_[record.location.siteId].push_back(index);
    for (const auto& p : record.participants) participantIndex_[p.stableId].push_back(index);
    for (const auto id : record.artifacts) artifactIndex_[id].push_back(index);
    for (const auto id : record.organizations) organizationIndex_[id].push_back(index);
}

void GalacticChronicle::applyEventToDurableSummaries(const HistoricalEventRecord& e) {
    if (e.location.siteId != 0) {
        if (auto it = sites_.find(e.location.siteId); it != sites_.end()) {
            switch (e.type) {
                case HistoryEventType::SiteClaimed: it->second.status = SiteStatus::Claimed; break;
                case HistoryEventType::SiteAbandoned: it->second.status = SiteStatus::Abandoned; break;
                case HistoryEventType::SiteDestroyed: it->second.status = SiteStatus::Destroyed; break;
                case HistoryEventType::SiteContaminated: it->second.status = SiteStatus::Contaminated; break;
                case HistoryEventType::SiteReclaimed:
                case HistoryEventType::FortressReclaimed: it->second.status = SiteStatus::Reclaimed; break;
                case HistoryEventType::FortressRetired: it->second.status = SiteStatus::Retired; break;
                default: break;
            }

            if (e.type == HistoryEventType::SiteClaimed || e.type == HistoryEventType::Occupation ||
                e.type == HistoryEventType::Liberation || e.type == HistoryEventType::Annexation ||
                e.type == HistoryEventType::SiteReclaimed) {
                const auto previous = payloadUint64(e, "previous_owner_org").value_or(it->second.ownerOrganizationId);
                auto next = payloadUint64(e, "new_owner_org").value_or(0);
                if (next == 0 && !e.organizations.empty()) next = e.organizations.back();
                if (next != 0) {
                    auto owningCivilization = [&](OrganizationId owner) -> OrganizationId {
                        if (civilizations_.contains(owner)) return owner;
                        if (const auto orgIt = organizations_.find(owner); orgIt != organizations_.end()) return orgIt->second.civilizationId;
                        return 0;
                    };
                    const auto previousCiv = owningCivilization(previous);
                    const auto nextCiv = owningCivilization(next);
                    if (previousCiv != 0 && previousCiv != nextCiv) {
                        auto& territory = civilizations_[previousCiv].territorySites;
                        territory.erase(std::remove(territory.begin(), territory.end(), e.location.siteId), territory.end());
                    }
                    if (nextCiv != 0) {
                        auto& territory = civilizations_[nextCiv].territorySites;
                        territory.push_back(e.location.siteId);
                        sortUnique(territory);
                    }
                    it->second.ownerOrganizationId = next;
                }
            }
        }
    }
    if ((e.type == HistoryEventType::Death || e.type == HistoryEventType::Disappearance) && !e.participants.empty()) {
        if (auto it = figures_.find(e.participants.front().stableId); it != figures_.end()) it->second.alive = false;
    }
    if ((e.type == HistoryEventType::ArtifactGifted || e.type == HistoryEventType::ArtifactInherited ||
         e.type == HistoryEventType::ArtifactRecovered || e.type == HistoryEventType::ArtifactStolen) &&
        !e.artifacts.empty() && !e.participants.empty()) {
        if (auto it = artifacts_.find(e.artifacts.front()); it != artifacts_.end()) {
            it->second.currentOwnerStableId = e.participants.back().stableId;
            if (e.location.siteId != 0) it->second.currentSiteId = e.location.siteId;
        }
    }
    if ((e.type == HistoryEventType::ArtifactLost || e.type == HistoryEventType::ArtifactDestroyed) && !e.artifacts.empty()) {
        if (auto it = artifacts_.find(e.artifacts.front()); it != artifacts_.end()) {
            it->second.currentOwnerStableId = 0;
            if (e.type == HistoryEventType::ArtifactDestroyed) it->second.currentSiteId = 0;
        }
    }
}

void GalacticChronicle::rebuildIndexes() {
    eventById_.clear(); participantIndex_.clear(); siteIndex_.clear(); artifactIndex_.clear(); organizationIndex_.clear(); typeIndex_.clear();
    for (std::size_t i = 0; i < events_.size(); ++i) indexEvent(i);
}

std::vector<HistoricalEventRecord> GalacticChronicle::query(const ChronicleQuery& q) const {
    std::vector<std::size_t> candidates;
    bool seeded = false;
    auto seedOrIntersect = [&](const std::vector<std::size_t>* idx) {
        if (!idx) { candidates.clear(); seeded = true; return; }
        if (!seeded) { candidates = *idx; seeded = true; }
        else candidates = intersectIndexes(candidates, *idx);
    };

    if (q.type) {
        const auto it = typeIndex_.find(*q.type); seedOrIntersect(it == typeIndex_.end() ? nullptr : &it->second);
    }
    if (q.siteId) {
        const auto it = siteIndex_.find(*q.siteId); seedOrIntersect(it == siteIndex_.end() ? nullptr : &it->second);
    }
    if (q.participantStableId) {
        const auto it = participantIndex_.find(*q.participantStableId); seedOrIntersect(it == participantIndex_.end() ? nullptr : &it->second);
    }
    if (q.artifactId) {
        const auto it = artifactIndex_.find(*q.artifactId); seedOrIntersect(it == artifactIndex_.end() ? nullptr : &it->second);
    }
    if (q.organizationId) {
        const auto it = organizationIndex_.find(*q.organizationId); seedOrIntersect(it == organizationIndex_.end() ? nullptr : &it->second);
    }
    if (!seeded) {
        candidates.resize(events_.size());
        for (std::size_t i = 0; i < events_.size(); ++i) candidates[i] = i;
    }

    std::vector<HistoricalEventRecord> out;
    for (const auto idx : candidates) {
        const auto& e = events_[idx];
        if (q.fromTimestamp && e.timestamp < *q.fromTimestamp) continue;
        if (q.toTimestamp && e.timestamp > *q.toTimestamp) continue;
        if (q.systemIndex && e.location.systemIndex != *q.systemIndex) continue;
        out.push_back(e);
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return std::tie(a.timestamp, a.eventId) < std::tie(b.timestamp, b.eventId);
    });
    return out;
}

std::vector<HistoricalEventRecord> GalacticChronicle::eventsForFigure(HistoryStableId id) const {
    ChronicleQuery q{}; q.participantStableId = id; return query(q);
}
std::vector<HistoricalEventRecord> GalacticChronicle::eventsForSite(SiteId id) const {
    ChronicleQuery q{}; q.siteId = id; return query(q);
}
std::vector<HistoricalEventRecord> GalacticChronicle::eventsForArtifact(ArtifactId id) const {
    ChronicleQuery q{}; q.artifactId = id; return query(q);
}
std::vector<HistoricalEventRecord> GalacticChronicle::eventsForOrganization(OrganizationId id) const {
    ChronicleQuery q{}; q.organizationId = id; return query(q);
}

std::vector<SiteOwnershipChange> GalacticChronicle::siteOwnershipTimeline(SiteId id) const {
    std::vector<SiteOwnershipChange> out;
    OrganizationId inferredOwner = 0;
    const auto timeline = eventsForSite(id);
    for (const auto& e : timeline) {
        if (e.type == HistoryEventType::SiteFounded) {
            if (!e.organizations.empty()) {
                const auto next = e.organizations.front();
                out.push_back({e.eventId, e.timestamp, inferredOwner, next, e.type});
                inferredOwner = next;
            }
            continue;
        }
        if (e.type != HistoryEventType::SiteClaimed && e.type != HistoryEventType::Occupation &&
            e.type != HistoryEventType::Liberation && e.type != HistoryEventType::Annexation &&
            e.type != HistoryEventType::SiteReclaimed) continue;
        const auto previous = payloadUint64(e, "previous_owner_org").value_or(inferredOwner);
        OrganizationId next = payloadUint64(e, "new_owner_org").value_or(0);
        if (next == 0 && !e.organizations.empty()) next = e.organizations.back();
        if (next == 0) continue;
        out.push_back({e.eventId, e.timestamp, previous, next, e.type});
        inferredOwner = next;
    }
    return out;
}

std::vector<ArtifactOwnershipChange> GalacticChronicle::artifactOwnershipTimeline(ArtifactId id) const {
    std::vector<ArtifactOwnershipChange> out;
    HistoryStableId inferredOwner = 0;
    const auto timeline = eventsForArtifact(id);
    for (const auto& e : timeline) {
        if (e.type == HistoryEventType::ArtifactCreated) {
            HistoryStableId next = 0;
            for (const auto& p : e.participants) if (p.role == "first_owner" || p.role == "owner") next = p.stableId;
            if (next == 0 && !e.participants.empty()) next = e.participants.back().stableId;
            out.push_back({e.eventId, e.timestamp, inferredOwner, next, e.type});
            inferredOwner = next;
            continue;
        }
        if (e.type != HistoryEventType::ArtifactGifted && e.type != HistoryEventType::ArtifactInherited &&
            e.type != HistoryEventType::ArtifactRecovered && e.type != HistoryEventType::ArtifactStolen &&
            e.type != HistoryEventType::ArtifactLost && e.type != HistoryEventType::ArtifactDestroyed) continue;
        HistoryStableId next = 0;
        if (e.type != HistoryEventType::ArtifactLost && e.type != HistoryEventType::ArtifactDestroyed && !e.participants.empty())
            next = e.participants.back().stableId;
        out.push_back({e.eventId, e.timestamp, inferredOwner, next, e.type});
        inferredOwner = next;
    }
    return out;
}

StrategicRelationSummary GalacticChronicle::relationBetween(OrganizationId left, OrganizationId right) const {
    StrategicRelationSummary out{};
    if (left == 0 || right == 0 || left == right) return out;
    if (right < left) std::swap(left, right);
    out.left = left;
    out.right = right;
    for (const auto& e : events_) {
        const auto stance = stanceFromEvent(e.type);
        if (!stance || e.organizations.size() < 2) continue;
        const bool hasLeft = std::find(e.organizations.begin(), e.organizations.end(), left) != e.organizations.end();
        const bool hasRight = std::find(e.organizations.begin(), e.organizations.end(), right) != e.organizations.end();
        if (!hasLeft || !hasRight) continue;
        if (out.lastEventId == 0 || std::tie(out.lastChangedTimestamp, out.lastEventId) < std::tie(e.timestamp, e.eventId)) {
            out.stance = *stance;
            out.lastEventId = e.eventId;
            out.lastChangedTimestamp = e.timestamp;
        }
    }
    return out;
}

std::vector<StrategicRelationSummary> GalacticChronicle::relationsForOrganization(OrganizationId id) const {
    std::set<OrganizationId> peers;
    for (const auto& e : eventsForOrganization(id)) {
        if (!stanceFromEvent(e.type)) continue;
        for (const auto org : e.organizations) if (org != id) peers.insert(org);
    }
    std::vector<StrategicRelationSummary> out;
    for (const auto peer : peers) {
        auto relation = relationBetween(id, peer);
        if (relation.lastEventId != 0) out.push_back(relation);
    }
    std::sort(out.begin(), out.end(), [id](const auto& a, const auto& b) {
        const auto ap = a.left == id ? a.right : a.left;
        const auto bp = b.left == id ? b.right : b.left;
        return ap < bp;
    });
    return out;
}

std::vector<OrganizationId> GalacticChronicle::organizationLineage(OrganizationId id) const {
    if (id == 0) return {};
    std::vector<OrganizationId> chain;
    std::set<OrganizationId> visited;
    OrganizationId current = id;
    while (current != 0 && visited.insert(current).second) {
        chain.push_back(current);
        OrganizationId parent = 0;
        for (const auto& e : eventsForOrganization(current)) {
            if (e.type != HistoryEventType::Schism) continue;
            const auto child = payloadUint64(e, "child_organization_id");
            const auto p = payloadUint64(e, "parent_organization_id");
            if (child && p && *child == current) {
                parent = *p;
                break;
            }
        }
        current = parent;
    }
    std::reverse(chain.begin(), chain.end());
    return chain;
}

std::vector<HistoricalEventRecord> GalacticChronicle::traceCauses(HistoricalEventId eventId, std::size_t maxDepth) const {
    std::vector<HistoricalEventRecord> out;
    std::set<HistoricalEventId> visited;
    std::vector<std::pair<HistoricalEventId, std::size_t>> frontier{{eventId, 0}};
    while (!frontier.empty()) {
        const auto [id, depth] = frontier.back(); frontier.pop_back();
        if (depth > maxDepth || !visited.insert(id).second) continue;
        const auto* e = event(id);
        if (!e) continue;
        out.push_back(*e);
        auto causes = e->causeRefs;
        std::sort(causes.rbegin(), causes.rend());
        for (const auto cause : causes) frontier.push_back({cause, depth + 1});
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return std::tie(a.timestamp, a.eventId) < std::tie(b.timestamp, b.eventId);
    });
    return out;
}

ChronicleComparison GalacticChronicle::compareEntities(HistoryStableId left, HistoryStableId right) const {
    const auto l = eventsForFigure(left);
    const auto r = eventsForFigure(right);
    std::vector<HistoricalEventId> li, ri;
    for (const auto& e : l) li.push_back(e.eventId);
    for (const auto& e : r) ri.push_back(e.eventId);
    sortUnique(li); sortUnique(ri);
    ChronicleComparison out{};
    std::set_difference(li.begin(), li.end(), ri.begin(), ri.end(), std::back_inserter(out.leftOnly));
    std::set_difference(ri.begin(), ri.end(), li.begin(), li.end(), std::back_inserter(out.rightOnly));
    std::set_intersection(li.begin(), li.end(), ri.begin(), ri.end(), std::back_inserter(out.shared));
    return out;
}

std::vector<ChronicleSearchHit> GalacticChronicle::search(std::string_view text) const {
    std::vector<ChronicleSearchHit> out;
    for (const auto& [id, r] : civilizations_) if (containsCaseInsensitive(r.name, text)) out.push_back({ChronicleSearchKind::Civilization, id, r.name});
    for (const auto& [id, r] : sites_) if (containsCaseInsensitive(r.name, text)) out.push_back({ChronicleSearchKind::Site, id, r.name});
    for (const auto& [id, r] : organizations_) if (containsCaseInsensitive(r.name, text)) out.push_back({ChronicleSearchKind::Organization, id, r.name});
    for (const auto& [id, r] : figures_) if (containsCaseInsensitive(r.name, text)) out.push_back({ChronicleSearchKind::HistoricalFigure, id, r.name});
    for (const auto& [id, r] : artifacts_) if (containsCaseInsensitive(r.name, text)) out.push_back({ChronicleSearchKind::Artifact, id, r.name});
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        return std::tie(a.name, a.kind, a.id) < std::tie(b.name, b.kind, b.id);
    });
    return out;
}

std::uint64_t GalacticChronicle::deterministicDigest() const {
    const auto text = serialize();
    return stableStringHash(text);
}

std::string GalacticChronicle::serialize() const {
    std::ostringstream out;
    out << "ELYSIUM_GALACTIC_CHRONICLE 1\n";
    out << "meta " << historySeed_ << ' ' << generatorVersion_ << ' ' << generatorFingerprint_ << ' ' << nextEventSerial_ << '\n';

    out << "civilizations " << civilizations_.size() << '\n';
    for (const auto& [id, r] : civilizations_) {
        out << "civilization " << id << ' ' << r.originSystem << ' ' << r.technologyLevel << ' ' << r.economicStrength << ' '
            << std::quoted(r.name) << ' ' << std::quoted(r.culture) << ' ' << std::quoted(r.government) << ' '
            << r.territorySites.size();
        for (const auto v : r.territorySites) out << ' ' << v;
        out << ' ' << r.organizations.size();
        for (const auto v : r.organizations) out << ' ' << v;
        out << '\n';
    }

    out << "sites " << sites_.size() << '\n';
    for (const auto& [id, r] : sites_) {
        out << "site " << id << ' ' << r.systemIndex << ' ' << r.planetIndex << ' ' << r.ownerOrganizationId << ' '
            << static_cast<int>(r.status) << ' ' << r.populationEstimate << ' ' << std::quoted(r.name) << '\n';
    }

    out << "organizations " << organizations_.size() << '\n';
    for (const auto& [id, r] : organizations_) {
        out << "organization " << id << ' ' << static_cast<int>(r.kind) << ' ' << r.civilizationId << ' ' << r.homeSiteId << ' '
            << r.strategicStrength << ' ' << r.readiness << ' ' << std::quoted(r.name) << ' ' << std::quoted(r.doctrine) << '\n';
    }

    out << "figures " << figures_.size() << '\n';
    for (const auto& [id, r] : figures_) {
        out << "figure " << id << ' ' << r.originSiteId << ' ' << r.currentSiteId << ' ' << r.organizationId << ' '
            << r.birthTimestamp << ' ' << r.significanceScore << ' ' << (r.alive ? 1 : 0) << ' ' << std::quoted(r.name) << '\n';
    }

    out << "artifacts " << artifacts_.size() << '\n';
    for (const auto& [id, r] : artifacts_) {
        out << "artifact " << id << ' ' << r.makerStableId << ' ' << r.currentOwnerStableId << ' ' << r.currentSiteId << ' '
            << r.creationEventId << ' ' << std::quoted(r.name) << '\n';
    }

    out << "events " << events_.size() << '\n';
    for (const auto& e : events_) {
        out << "event " << e.eventId << ' ' << e.timestamp << ' ' << static_cast<int>(e.type) << ' '
            << e.location.systemIndex << ' ' << e.location.planetIndex << ' ' << e.location.siteId << ' '
            << e.knowledgeVisibility << ' ' << static_cast<unsigned>(e.importance) << ' '
            << e.participants.size() << ' ' << e.artifacts.size() << ' ' << e.organizations.size() << ' '
            << e.causeRefs.size() << ' ' << e.payload.size() << '\n';
        for (const auto& p : e.participants) out << "participant " << p.stableId << ' ' << std::quoted(p.role) << '\n';
        for (const auto id : e.artifacts) out << "artifact_ref " << id << '\n';
        for (const auto id : e.organizations) out << "organization_ref " << id << '\n';
        for (const auto id : e.causeRefs) out << "cause_ref " << id << '\n';
        for (const auto& field : e.payload) out << "payload " << std::quoted(field.key) << ' ' << std::quoted(field.value) << '\n';
        out << "end_event\n";
    }
    out << "end_chronicle\n";
    return out.str();
}

bool GalacticChronicle::restore(std::string_view text, std::string* error) {
    auto fail = [&](std::string message) { if (error) *error = std::move(message); return false; };
    std::istringstream in{std::string(text)};
    std::string tag;
    int schema{};
    if (!(in >> tag >> schema) || tag != "ELYSIUM_GALACTIC_CHRONICLE" || schema != 1) return fail("unsupported chronicle header");

    GalacticChronicle restored;
    if (!(in >> tag >> restored.historySeed_ >> restored.generatorVersion_ >> restored.generatorFingerprint_ >> restored.nextEventSerial_) ||
        tag != "meta" || restored.nextEventSerial_ == 0) return fail("malformed chronicle metadata");

    std::size_t count{};
    if (!(in >> tag >> count) || tag != "civilizations" || count > 1000000) return fail("malformed civilization section");
    for (std::size_t i = 0; i < count; ++i) {
        CivilizationRecord r{}; std::size_t territoryCount{}, orgCount{};
        if (!(in >> tag >> r.id >> r.originSystem >> r.technologyLevel >> r.economicStrength >> std::quoted(r.name) >> std::quoted(r.culture) >> std::quoted(r.government) >> territoryCount) ||
            tag != "civilization" || territoryCount > 1000000) return fail("malformed civilization record");
        r.territorySites.resize(territoryCount); for (auto& v : r.territorySites) if (!(in >> v)) return fail("malformed civilization territory");
        if (!(in >> orgCount) || orgCount > 1000000) return fail("malformed civilization organizations");
        r.organizations.resize(orgCount); for (auto& v : r.organizations) if (!(in >> v)) return fail("malformed civilization organization reference");
        std::string localError; if (!restored.registerCivilization(std::move(r), &localError)) return fail(localError);
    }

    if (!(in >> tag >> count) || tag != "sites" || count > 1000000) return fail("malformed site section");
    for (std::size_t i = 0; i < count; ++i) {
        SiteRecord r{}; int status{};
        if (!(in >> tag >> r.id >> r.systemIndex >> r.planetIndex >> r.ownerOrganizationId >> status >> r.populationEstimate >> std::quoted(r.name)) ||
            tag != "site" || !validSiteStatus(status)) return fail("malformed site record");
        r.status = static_cast<SiteStatus>(status);
        std::string localError; if (!restored.registerSite(std::move(r), &localError)) return fail(localError);
    }

    if (!(in >> tag >> count) || tag != "organizations" || count > 1000000) return fail("malformed organization section");
    for (std::size_t i = 0; i < count; ++i) {
        OrganizationRecord r{}; int kind{};
        if (!(in >> tag >> r.id >> kind >> r.civilizationId >> r.homeSiteId >> r.strategicStrength >> r.readiness >> std::quoted(r.name) >> std::quoted(r.doctrine)) ||
            tag != "organization" || !validOrganizationKind(kind)) return fail("malformed organization record");
        r.kind = static_cast<OrganizationKind>(kind);
        std::string localError; if (!restored.registerOrganization(std::move(r), &localError)) return fail(localError);
    }

    if (!(in >> tag >> count) || tag != "figures" || count > 1000000) return fail("malformed figure section");
    for (std::size_t i = 0; i < count; ++i) {
        HistoricalFigureRecord r{}; int alive{};
        if (!(in >> tag >> r.stableId >> r.originSiteId >> r.currentSiteId >> r.organizationId >> r.birthTimestamp >> r.significanceScore >> alive >> std::quoted(r.name)) ||
            tag != "figure" || (alive != 0 && alive != 1)) return fail("malformed figure record");
        r.alive = alive != 0;
        std::string localError; if (!restored.registerFigure(std::move(r), &localError)) return fail(localError);
    }

    if (!(in >> tag >> count) || tag != "artifacts" || count > 1000000) return fail("malformed artifact section");
    for (std::size_t i = 0; i < count; ++i) {
        ArtifactRecord r{};
        if (!(in >> tag >> r.id >> r.makerStableId >> r.currentOwnerStableId >> r.currentSiteId >> r.creationEventId >> std::quoted(r.name)) || tag != "artifact")
            return fail("malformed artifact record");
        std::string localError; if (!restored.registerArtifact(std::move(r), &localError)) return fail(localError);
    }

    if (!(in >> tag >> count) || tag != "events" || count > 10000000) return fail("malformed event section");
    restored.events_.reserve(count);
    std::set<HistoricalEventId> seenIds;
    for (std::size_t i = 0; i < count; ++i) {
        HistoricalEventRecord e{}; int type{}; unsigned importance{};
        std::size_t participantCount{}, artifactCount{}, organizationCount{}, causeCount{}, payloadCount{};
        if (!(in >> tag >> e.eventId >> e.timestamp >> type >> e.location.systemIndex >> e.location.planetIndex >> e.location.siteId >>
              e.knowledgeVisibility >> importance >> participantCount >> artifactCount >> organizationCount >> causeCount >> payloadCount) ||
            tag != "event" || e.eventId == 0 || !validEventType(type) || importance > 100 || participantCount > 100000 || artifactCount > 100000 ||
            organizationCount > 100000 || causeCount > 100000 || payloadCount > 100000 || !seenIds.insert(e.eventId).second)
            return fail("malformed or duplicate historical event");
        e.type = static_cast<HistoryEventType>(type); e.importance = static_cast<std::uint8_t>(importance);
        for (std::size_t j = 0; j < participantCount; ++j) { HistoryParticipant p{}; if (!(in >> tag >> p.stableId >> std::quoted(p.role)) || tag != "participant" || p.stableId == 0) return fail("malformed participant reference"); e.participants.push_back(std::move(p)); }
        for (std::size_t j = 0; j < artifactCount; ++j) { ArtifactId id{}; if (!(in >> tag >> id) || tag != "artifact_ref" || id == 0) return fail("malformed artifact reference"); e.artifacts.push_back(id); }
        for (std::size_t j = 0; j < organizationCount; ++j) { OrganizationId id{}; if (!(in >> tag >> id) || tag != "organization_ref" || id == 0) return fail("malformed organization reference"); e.organizations.push_back(id); }
        for (std::size_t j = 0; j < causeCount; ++j) { HistoricalEventId id{}; if (!(in >> tag >> id) || tag != "cause_ref" || id == 0 || !seenIds.contains(id)) return fail("chronicle cause must refer to an earlier event"); e.causeRefs.push_back(id); }
        for (std::size_t j = 0; j < payloadCount; ++j) { HistoryPayloadField field{}; if (!(in >> tag >> std::quoted(field.key) >> std::quoted(field.value)) || tag != "payload") return fail("malformed event payload"); e.payload.push_back(std::move(field)); }
        if (!(in >> tag) || tag != "end_event") return fail("historical event missing terminator");
        canonicalizeDraft(e);
        restored.events_.push_back(std::move(e));
    }
    if (!(in >> tag) || tag != "end_chronicle") return fail("chronicle missing terminator");

    restored.rebuildIndexes();
    // Rebuild semantic summaries from the event stream so derived status is not
    // trusted from any serialized query index.
    for (const auto& e : restored.events_) restored.applyEventToDurableSummaries(e);
    *this = std::move(restored);
    if (error) error->clear();
    return true;
}

WorldHistoryGenerator::WorldHistoryGenerator(WorldHistoryGenerationConfig config) : config_(config) {
    if (config_.civilizationCount == 0) config_.civilizationCount = 1;
}

GalacticChronicle WorldHistoryGenerator::generateFoundation(std::vector<HistoryOpportunity> opportunities) const {
    std::sort(opportunities.begin(), opportunities.end(), [](const auto& a, const auto& b) {
        return std::tie(a.systemIndex, a.planetIndex, a.physicalSiteKey) < std::tie(b.systemIndex, b.planetIndex, b.physicalSiteKey);
    });
    opportunities.erase(std::unique(opportunities.begin(), opportunities.end(), [](const auto& a, const auto& b) {
        return a.physicalSiteKey == b.physicalSiteKey;
    }), opportunities.end());

    GalacticChronicle chronicle(config_.galaxySeed ^ kCivilizationLabel, config_.historyVersion, config_.historyFingerprint);
    if (opportunities.empty()) return chronicle;

    const auto civCount = std::min<std::uint32_t>(config_.civilizationCount, static_cast<std::uint32_t>(opportunities.size()));
    for (std::uint32_t c = 0; c < civCount; ++c) {
        const auto civId = derivedId(config_.galaxySeed, kCivilizationLabel, c + 1U, config_.historyVersion);
        CivilizationRecord civ{};
        civ.id = civId;
        civ.name = generatedName(civId, "Civilization", c);
        civ.culture = generatedName(civId ^ 0xC0117EULL, "Culture", c);
        civ.government = (c % 3U == 0U ? "Chartered council" : (c % 3U == 1U ? "Distributed covenant" : "Prefectural compact"));
        civ.originSystem = opportunities[c].systemIndex;
        civ.technologyLevel = static_cast<std::uint16_t>(1U + ((mix64(civId) >> 12U) % 5U));
        civ.economicStrength = static_cast<std::uint16_t>(25U + ((mix64(civId ^ 0xEC0ULL) >> 8U) % 76U));

        for (std::size_t oi = c; oi < opportunities.size(); oi += civCount) {
            const auto& opp = opportunities[oi];
            const auto siteId = derivedId(config_.galaxySeed, kSiteLabel, opp.physicalSiteKey, civId);
            SiteRecord site{};
            site.id = siteId;
            site.name = generatedName(siteId, "Site", static_cast<std::uint32_t>(oi));
            site.systemIndex = opp.systemIndex;
            site.planetIndex = opp.planetIndex;
            site.ownerOrganizationId = civId;
            site.populationEstimate = 40U + static_cast<std::uint32_t>((opp.habitability + opp.resourceRichness) / 2U);
            chronicle.registerSite(site);
            civ.territorySites.push_back(siteId);

            HistoricalEventDraft founded{};
            founded.timestamp = 0;
            founded.type = HistoryEventType::SiteFounded;
            founded.location = {opp.systemIndex, opp.planetIndex, siteId};
            founded.organizations = {civId};
            founded.importance = 60;
            founded.payload = {{"physical_site_key", std::to_string(opp.physicalSiteKey)}};
            chronicle.append(std::move(founded));
        }

        const SiteId homeSite = civ.territorySites.empty() ? 0 : civ.territorySites.front();
        constexpr OrganizationKind kinds[] = {OrganizationKind::LocalGovernment, OrganizationKind::TradeNetwork,
                                               OrganizationKind::Faith, OrganizationKind::MilitaryFormation};
        for (std::uint32_t k = 0; k < 4; ++k) {
            OrganizationRecord org{};
            org.id = derivedId(config_.galaxySeed, kOrganizationLabel, civId, k + 1U);
            org.kind = kinds[k];
            org.name = generatedName(org.id, organizationKindName(org.kind), k);
            org.civilizationId = civId;
            org.homeSiteId = homeSite;
            org.doctrine = generatedName(org.id ^ 0xD0C7ULL, "Doctrine", k);
            org.strategicStrength = static_cast<std::uint16_t>(20U + ((mix64(org.id) >> 7U) % 81U));
            org.readiness = static_cast<std::uint16_t>(20U + ((mix64(org.id ^ 0xA11ULL) >> 9U) % 81U));
            chronicle.registerOrganization(org);
            civ.organizations.push_back(org.id);

            HistoricalEventDraft orgFounded{};
            orgFounded.timestamp = 1;
            orgFounded.type = HistoryEventType::OrganizationFounded;
            if (const auto* s = chronicle.site(homeSite)) orgFounded.location = {s->systemIndex, s->planetIndex, homeSite};
            orgFounded.organizations = {civId, org.id};
            orgFounded.importance = 50;
            orgFounded.payload = {{"kind", organizationKindName(org.kind)}};
            chronicle.append(std::move(orgFounded));
        }
        sortUnique(civ.territorySites);
        sortUnique(civ.organizations);
        chronicle.registerCivilization(std::move(civ));
    }

    return chronicle;
}

void WorldHistoryGenerator::simulateEpoch(JobSystem& jobs,
                                          GalacticChronicle& chronicle,
                                          std::uint32_t epochIndex,
                                          std::uint64_t epochStartTimestamp) const {
    std::vector<OrganizationId> civIds;
    // The generator's civilization IDs can be recovered from the deterministic
    // foundation labels. Missing IDs are skipped so a partially loaded/history-
    // filtered Chronicle can still advance deterministically for what it owns.
    for (std::uint32_t c = 0; c < config_.civilizationCount; ++c) {
        const auto id = derivedId(config_.galaxySeed, kCivilizationLabel, c + 1U, config_.historyVersion);
        if (chronicle.civilization(id)) civIds.push_back(id);
    }
    std::sort(civIds.begin(), civIds.end());
    if (civIds.empty()) return;

    struct Candidate { OrganizationId civId{}; std::uint64_t key{}; HistoricalEventDraft draft; };
    std::vector<Candidate> candidates(civIds.size());
    jobs.parallelFor(civIds.size(), [&](std::size_t i) {
        const auto civId = civIds[i];
        const auto* civ = chronicle.civilization(civId);
        if (!civ) return;
        const auto h = mix64(config_.galaxySeed ^ kEpochLabel ^ civId ^ (static_cast<std::uint64_t>(epochIndex) << 32U));
        Candidate candidate{};
        candidate.civId = civId;
        candidate.key = h;
        candidate.draft.timestamp = epochStartTimestamp + (h % 997U);
        candidate.draft.importance = static_cast<std::uint8_t>(30U + (h % 51U));
        candidate.draft.organizations.push_back(civId);
        if (!civ->territorySites.empty()) {
            const auto siteId = civ->territorySites[static_cast<std::size_t>((h >> 8U) % civ->territorySites.size())];
            if (const auto* s = chronicle.site(siteId)) candidate.draft.location = {s->systemIndex, s->planetIndex, siteId};
        }
        switch ((h >> 17U) % 8U) {
            case 0: candidate.draft.type = HistoryEventType::Migration; break;
            case 1: candidate.draft.type = HistoryEventType::Discovery; break;
            case 2: candidate.draft.type = HistoryEventType::SiteExpanded; break;
            case 3: candidate.draft.type = HistoryEventType::ScientificBreakthrough; break;
            case 4: candidate.draft.type = HistoryEventType::Embargo; break;
            case 5: candidate.draft.type = HistoryEventType::Treaty; break;
            case 6: candidate.draft.type = HistoryEventType::WarDeclared; break;
            default: candidate.draft.type = HistoryEventType::ExpeditionResult; break;
        }
        if ((candidate.draft.type == HistoryEventType::Treaty || candidate.draft.type == HistoryEventType::WarDeclared || candidate.draft.type == HistoryEventType::Embargo) && civIds.size() > 1) {
            const auto other = civIds[(i + 1U + static_cast<std::size_t>((h >> 24U) % (civIds.size() - 1U))) % civIds.size()];
            if (other != civId) candidate.draft.organizations.push_back(other);
        }
        candidate.draft.payload = {{"epoch", std::to_string(epochIndex)}, {"event_seed", std::to_string(h)}};
        candidates[i] = std::move(candidate);
    });

    // Worker results are committed only after a deterministic owner-thread sort;
    // output therefore does not depend on worker count, lane or completion order.
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        return std::tie(a.draft.timestamp, a.civId, a.key) < std::tie(b.draft.timestamp, b.civId, b.key);
    });
    for (auto& candidate : candidates) if (candidate.civId != 0) chronicle.append(std::move(candidate.draft));
}

} // namespace elysium
