// Intended function: imported world implementation for WorldMemoryPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/WorldMemoryPersistence.hpp"

#include "world/SurfaceChunkPersistence.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::size_t kMaxRecords = 1'000'000;
constexpr std::size_t kMaxRefsPerRecord = 65'536;
constexpr std::size_t kMaxComponentPayloadBytes = 8 * 1024 * 1024;
constexpr std::size_t kMaxEventPayloadBytes = 8 * 1024 * 1024;

void setError(std::string* error, std::string message) {
    if(error) *error=std::move(message);
}

bool validContentId(std::string_view id) {
    if(id.empty() || id.size()>512 || id.find(':')==std::string_view::npos) return false;
    return std::none_of(id.begin(),id.end(),[](unsigned char c){return c<=0x20 || c==0x7f;});
}

bool validComponentId(std::string_view id) {
    return validContentId(id);
}

bool validChunk(const PlanetChunkAddress& chunk) {
    const int face=static_cast<int>(chunk.face);
    return face>=0 && face<6 && chunk.u>=0 && chunk.v>=0 && chunk.radial>=0;
}

bool validEntityKind(PersistentEntityKind kind) {
    return static_cast<int>(kind)<=static_cast<int>(PersistentEntityKind::Artifact);
}

bool validShard(PersistentShardState state) {
    return static_cast<int>(state)<=static_cast<int>(PersistentShardState::GalaxyHistorical);
}

bool validKnowledge(ChronicleKnowledge state) {
    return static_cast<int>(state)<=static_cast<int>(ChronicleKnowledge::Secret);
}

template<class T>
bool uniqueNonZero(const std::vector<T>& values) {
    std::unordered_set<T> seen;
    seen.reserve(values.size());
    for(const auto value:values) if(value==0 || !seen.insert(value).second) return false;
    return true;
}

void writeChunk(std::ostream& out,const std::optional<PlanetChunkAddress>& chunk) {
    if(!chunk) { out<<0; return; }
    out<<1<<' '<<static_cast<int>(chunk->face)<<' '<<chunk->u<<' '<<chunk->v<<' '<<chunk->radial;
}

bool readChunk(std::istream& in,std::optional<PlanetChunkAddress>& chunk) {
    int has{},face{},u{},v{},radial{};
    if(!(in>>has) || (has!=0 && has!=1)) return false;
    if(!has) {chunk.reset();return true;}
    if(!(in>>face>>u>>v>>radial) || face<0 || face>=6 || u<0 || v<0 || radial<0) return false;
    chunk=PlanetChunkAddress{static_cast<CubeFace>(face),u,v,radial};
    return true;
}

bool countOk(std::size_t count,std::size_t max=kMaxRefsPerRecord) { return count<=max; }

WorldMemoryLoadResult parseSnapshotImpl(std::string_view payload) {
    WorldMemoryLoadResult result{};
    std::istringstream in{std::string(payload)};
    std::string tag;
    WorldMemorySnapshot snapshot{};
    if(!(in>>tag>>snapshot.schemaVersion) || tag!="ELYSIUM_WORLD_MEMORY" || snapshot.schemaVersion!=WorldMemorySnapshot::SchemaVersion) {
        result.error="unsupported world-memory schema"; return result;
    }
    if(!(in>>tag>>snapshot.saveGeneration) || tag!="save_generation" || snapshot.saveGeneration==0) {
        result.error="invalid world-memory save generation"; return result;
    }

    std::size_t count{};
    if(!(in>>tag>>count) || tag!="baselines" || !countOk(count,kMaxRecords)) {result.error="invalid baseline section";return result;}
    snapshot.baselines.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        SiteBaselineRecord r{};
        if(!(in>>tag>>r.siteId>>r.planetSeed>>r.generatorVersion>>r.generatorFingerprint) || tag!="baseline") {result.error="invalid baseline record";return result;}
        snapshot.baselines.push_back(r);
    }

    if(!(in>>tag>>count) || tag!="entities" || !countOk(count,kMaxRecords)) {result.error="invalid entity section";return result;}
    snapshot.entities.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        PersistentEntityRecord r{}; int kind{},shard{}; int hasChunk{};
        if(!(in>>tag>>r.schemaVersion>>r.stableId>>kind>>r.location.systemIndex>>r.location.planetIndex>>r.location.siteId) || tag!="entity") {result.error="invalid entity header";return result;}
        // Chunk is read separately so optionality is explicit and versionable.
        if(!readChunk(in,r.location.chunk) || !(in>>r.location.x>>r.location.y>>r.location.z>>shard)) {result.error="invalid entity location";return result;}
        (void)hasChunk;
        if(kind<0 || kind>static_cast<int>(PersistentEntityKind::Artifact) || shard<0 || shard>static_cast<int>(PersistentShardState::GalaxyHistorical)) {result.error="entity enum out of range";return result;}
        r.kind=static_cast<PersistentEntityKind>(kind); r.shardState=static_cast<PersistentShardState>(shard);

        std::size_t refs{};
        if(!(in>>tag>>refs) || tag!="content_refs" || !countOk(refs)) {result.error="invalid entity content refs";return result;}
        r.contentRefs.reserve(refs);
        for(std::size_t j=0;j<refs;++j) {
            PersistentContentRef ref{};
            if(!(in>>tag>>std::quoted(ref.id)>>ref.schemaVersion) || tag!="content") {result.error="invalid content ref";return result;}
            r.contentRefs.push_back(std::move(ref));
        }
        if(!(in>>tag>>refs) || tag!="components" || !countOk(refs)) {result.error="invalid entity components";return result;}
        r.components.reserve(refs);
        for(std::size_t j=0;j<refs;++j) {
            PersistentComponentBlob blob{};
            if(!(in>>tag>>std::quoted(blob.componentId)>>blob.schemaVersion>>std::quoted(blob.payload)) || tag!="component" || blob.payload.size()>kMaxComponentPayloadBytes) {result.error="invalid component blob";return result;}
            r.components.push_back(std::move(blob));
        }
        if(!(in>>tag>>refs) || tag!="relationships" || !countOk(refs)) {result.error="invalid relationship refs";return result;}
        r.relationshipRefs.resize(refs);
        for(auto& id:r.relationshipRefs) if(!(in>>id)) {result.error="invalid relationship ref";return result;}
        if(!(in>>tag>>refs) || tag!="history" || !countOk(refs)) {result.error="invalid history refs";return result;}
        r.historyRefs.resize(refs);
        for(auto& id:r.historyRefs) if(!(in>>id)) {result.error="invalid history ref";return result;}
        snapshot.entities.push_back(std::move(r));
    }

    if(!(in>>tag>>count) || tag!="events" || !countOk(count,kMaxRecords)) {result.error="invalid event section";return result;}
    snapshot.events.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        ChronicleEventRecord e{}; int knowledge{},importance{};
        if(!(in>>tag>>e.schemaVersion>>e.eventId>>e.timestamp>>std::quoted(e.typeId)
              >>e.location.systemIndex>>e.location.planetIndex>>e.location.siteId) || tag!="event") {result.error="invalid event header";return result;}
        if(!readChunk(in,e.location.chunk) || !(in>>knowledge>>importance) || knowledge<0 || knowledge>static_cast<int>(ChronicleKnowledge::Secret) || importance<0 || importance>255) {result.error="invalid event location/state";return result;}
        e.knowledge=static_cast<ChronicleKnowledge>(knowledge); e.importance=static_cast<std::uint8_t>(importance);

        std::size_t refs{};
        if(!(in>>tag>>refs) || tag!="participants" || !countOk(refs)) {result.error="invalid event participants";return result;}
        e.participants.reserve(refs);
        for(std::size_t j=0;j<refs;++j) {
            ChronicleParticipant p{};
            if(!(in>>tag>>p.stableId>>std::quoted(p.role)) || tag!="participant") {result.error="invalid event participant";return result;}
            e.participants.push_back(std::move(p));
        }
        if(!(in>>tag>>refs) || tag!="objects" || !countOk(refs)) {result.error="invalid event objects";return result;}
        e.objectRefs.resize(refs); for(auto& id:e.objectRefs) if(!(in>>id)){result.error="invalid object ref";return result;}
        if(!(in>>tag>>refs) || tag!="organizations" || !countOk(refs)) {result.error="invalid event organizations";return result;}
        e.organizationRefs.resize(refs); for(auto& id:e.organizationRefs) if(!(in>>id)){result.error="invalid organization ref";return result;}
        if(!(in>>tag>>refs) || tag!="causes" || !countOk(refs)) {result.error="invalid event causes";return result;}
        e.causeRefs.resize(refs); for(auto& id:e.causeRefs) if(!(in>>id)){result.error="invalid cause ref";return result;}
        if(!(in>>tag>>std::quoted(e.payload)) || tag!="payload" || e.payload.size()>kMaxEventPayloadBytes) {result.error="invalid event payload";return result;}
        snapshot.events.push_back(std::move(e));
    }
    if(!(in>>tag) || tag!="END") {result.error="missing world-memory END";return result;}
    std::string validationError;
    if(!validateWorldMemorySnapshot(snapshot,&validationError)) {result.error=validationError;return result;}
    result.loaded=true;
    result.snapshot=std::move(snapshot);
    return result;
}

} // namespace

bool ContentIdAliasTable::addAlias(std::string from,std::string to,std::string* error) {
    if(!validContentId(from) || !validContentId(to)) {setError(error,"content alias IDs must be namespaced non-whitespace identifiers");return false;}
    if(from==to) {setError(error,"content alias cannot point to itself");return false;}
    if(const auto it=aliases_.find(from);it!=aliases_.end()) {
        if(it->second==to) return true;
        setError(error,"content alias source already maps to another target");return false;
    }
    // Reject a cycle before publishing the edge.
    std::string cursor=to;
    std::unordered_set<std::string> seen{from};
    for(std::size_t i=0;i<=aliases_.size();++i) {
        if(!seen.insert(cursor).second) {setError(error,"content alias would create a cycle");return false;}
        const auto it=aliases_.find(cursor);
        if(it==aliases_.end()) break;
        cursor=it->second;
    }
    aliases_.emplace(std::move(from),std::move(to));
    return true;
}

std::optional<std::string> ContentIdAliasTable::resolve(std::string_view id,std::string* error) const {
    if(!validContentId(id)) {setError(error,"invalid content ID");return std::nullopt;}
    std::string cursor(id);
    std::unordered_set<std::string> seen;
    for(std::size_t i=0;i<=aliases_.size();++i) {
        if(!seen.insert(cursor).second) {setError(error,"content alias cycle encountered");return std::nullopt;}
        const auto it=aliases_.find(cursor);
        if(it==aliases_.end()) return cursor;
        cursor=it->second;
    }
    setError(error,"content alias chain exceeds table size");
    return std::nullopt;
}

bool validateWorldMemorySnapshot(const WorldMemorySnapshot& snapshot,std::string* error) {
    if(snapshot.schemaVersion!=WorldMemorySnapshot::SchemaVersion) {setError(error,"unsupported world-memory snapshot schema");return false;}
    if(snapshot.saveGeneration==0) {setError(error,"world-memory save generation must be nonzero");return false;}
    if(snapshot.baselines.size()>kMaxRecords || snapshot.entities.size()>kMaxRecords || snapshot.events.size()>kMaxRecords) {setError(error,"world-memory record count exceeds safety limit");return false;}

    std::unordered_set<SiteId> sites;
    for(const auto& b:snapshot.baselines) {
        if(b.siteId==0 || b.generatorVersion<=0 || b.generatorFingerprint==0 || !sites.insert(b.siteId).second) {setError(error,"invalid or duplicate site baseline record");return false;}
    }

    std::unordered_set<EntityStableId> entities;
    for(const auto& r:snapshot.entities) {
        if(r.schemaVersion!=PersistentEntityRecord::SchemaVersion || r.stableId==0 || !entities.insert(r.stableId).second || !validEntityKind(r.kind) || !validShard(r.shardState)) {setError(error,"invalid or duplicate persistent entity record");return false;}
        if(r.location.chunk && !validChunk(*r.location.chunk)) {setError(error,"persistent entity has invalid chunk address");return false;}
        if(!std::isfinite(r.location.x) || !std::isfinite(r.location.y) || !std::isfinite(r.location.z)) {setError(error,"persistent entity location is not finite");return false;}
        if(r.contentRefs.size()>kMaxRefsPerRecord || r.components.size()>kMaxRefsPerRecord || r.relationshipRefs.size()>kMaxRefsPerRecord || r.historyRefs.size()>kMaxRefsPerRecord) {setError(error,"persistent entity reference count exceeds safety limit");return false;}
        std::unordered_set<std::string> contentIds;
        for(const auto& ref:r.contentRefs) if(!validContentId(ref.id) || ref.schemaVersion==0 || !contentIds.insert(ref.id).second) {setError(error,"invalid or duplicate persistent content reference");return false;}
        std::unordered_set<std::string> componentIds;
        for(const auto& blob:r.components) if(!validComponentId(blob.componentId) || blob.schemaVersion==0 || blob.payload.size()>kMaxComponentPayloadBytes || !componentIds.insert(blob.componentId).second) {setError(error,"invalid or duplicate persistent component blob");return false;}
        if(!uniqueNonZero(r.relationshipRefs) || !uniqueNonZero(r.historyRefs)) {setError(error,"persistent entity has zero/duplicate relationship or history reference");return false;}
    }

    std::unordered_set<HistoricalEventId> events;
    for(const auto& e:snapshot.events) {
        if(e.schemaVersion!=ChronicleEventRecord::SchemaVersion || e.eventId==0 || !events.insert(e.eventId).second || !validContentId(e.typeId) || !validKnowledge(e.knowledge)) {setError(error,"invalid or duplicate Chronicle event record");return false;}
        if(e.location.chunk && !validChunk(*e.location.chunk)) {setError(error,"Chronicle event has invalid chunk address");return false;}
        if(e.participants.size()>kMaxRefsPerRecord || e.objectRefs.size()>kMaxRefsPerRecord || e.organizationRefs.size()>kMaxRefsPerRecord || e.causeRefs.size()>kMaxRefsPerRecord || e.payload.size()>kMaxEventPayloadBytes) {setError(error,"Chronicle event reference/payload limit exceeded");return false;}
        std::unordered_set<EntityStableId> participantIds;
        for(const auto& p:e.participants) if(p.stableId==0 || p.role.empty() || !participantIds.insert(p.stableId).second) {setError(error,"Chronicle event has invalid/duplicate participant");return false;}
        if(!uniqueNonZero(e.objectRefs) || !uniqueNonZero(e.organizationRefs) || !uniqueNonZero(e.causeRefs)) {setError(error,"Chronicle event has zero/duplicate object, organization, or cause reference");return false;}
        if(std::find(e.causeRefs.begin(),e.causeRefs.end(),e.eventId)!=e.causeRefs.end()) {setError(error,"Chronicle event cannot cause itself");return false;}
    }
    return true;
}

std::string serializeWorldMemorySnapshot(const WorldMemorySnapshot& snapshot) {
    std::string error;
    if(!validateWorldMemorySnapshot(snapshot,&error)) return {};

    auto baselines=snapshot.baselines;
    auto entities=snapshot.entities;
    auto events=snapshot.events;
    std::sort(baselines.begin(),baselines.end(),[](const auto& a,const auto& b){return a.siteId<b.siteId;});
    std::sort(entities.begin(),entities.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    std::sort(events.begin(),events.end(),[](const auto& a,const auto& b){return a.eventId<b.eventId;});

    std::ostringstream out;
    out<<std::setprecision(17);
    out<<"ELYSIUM_WORLD_MEMORY "<<snapshot.schemaVersion<<"\n";
    out<<"save_generation "<<snapshot.saveGeneration<<"\n";
    out<<"baselines "<<baselines.size()<<"\n";
    for(const auto& b:baselines) out<<"baseline "<<b.siteId<<' '<<b.planetSeed<<' '<<b.generatorVersion<<' '<<b.generatorFingerprint<<"\n";

    out<<"entities "<<entities.size()<<"\n";
    for(auto r:entities) {
        std::sort(r.contentRefs.begin(),r.contentRefs.end(),[](const auto& a,const auto& b){return a.id<b.id;});
        std::sort(r.components.begin(),r.components.end(),[](const auto& a,const auto& b){return a.componentId<b.componentId;});
        std::sort(r.relationshipRefs.begin(),r.relationshipRefs.end());
        std::sort(r.historyRefs.begin(),r.historyRefs.end());
        out<<"entity "<<r.schemaVersion<<' '<<r.stableId<<' '<<static_cast<int>(r.kind)<<' '
           <<r.location.systemIndex<<' '<<r.location.planetIndex<<' '<<r.location.siteId<<' ';
        writeChunk(out,r.location.chunk);
        out<<' '<<r.location.x<<' '<<r.location.y<<' '<<r.location.z<<' '<<static_cast<int>(r.shardState)<<"\n";
        out<<"content_refs "<<r.contentRefs.size()<<"\n";
        for(const auto& ref:r.contentRefs) out<<"content "<<std::quoted(ref.id)<<' '<<ref.schemaVersion<<"\n";
        out<<"components "<<r.components.size()<<"\n";
        for(const auto& blob:r.components) out<<"component "<<std::quoted(blob.componentId)<<' '<<blob.schemaVersion<<' '<<std::quoted(blob.payload)<<"\n";
        out<<"relationships "<<r.relationshipRefs.size(); for(const auto id:r.relationshipRefs) out<<' '<<id; out<<"\n";
        out<<"history "<<r.historyRefs.size(); for(const auto id:r.historyRefs) out<<' '<<id; out<<"\n";
    }

    out<<"events "<<events.size()<<"\n";
    for(auto e:events) {
        std::sort(e.participants.begin(),e.participants.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
        std::sort(e.objectRefs.begin(),e.objectRefs.end());
        std::sort(e.organizationRefs.begin(),e.organizationRefs.end());
        std::sort(e.causeRefs.begin(),e.causeRefs.end());
        out<<"event "<<e.schemaVersion<<' '<<e.eventId<<' '<<e.timestamp<<' '<<std::quoted(e.typeId)<<' '
           <<e.location.systemIndex<<' '<<e.location.planetIndex<<' '<<e.location.siteId<<' ';
        writeChunk(out,e.location.chunk);
        out<<' '<<static_cast<int>(e.knowledge)<<' '<<static_cast<int>(e.importance)<<"\n";
        out<<"participants "<<e.participants.size()<<"\n";
        for(const auto& p:e.participants) out<<"participant "<<p.stableId<<' '<<std::quoted(p.role)<<"\n";
        out<<"objects "<<e.objectRefs.size(); for(const auto id:e.objectRefs) out<<' '<<id; out<<"\n";
        out<<"organizations "<<e.organizationRefs.size(); for(const auto id:e.organizationRefs) out<<' '<<id; out<<"\n";
        out<<"causes "<<e.causeRefs.size(); for(const auto id:e.causeRefs) out<<' '<<id; out<<"\n";
        out<<"payload "<<std::quoted(e.payload)<<"\n";
    }
    out<<"END\n";
    return out.str();
}

WorldMemoryLoadResult parseWorldMemorySnapshot(std::string_view payload) {
    return parseSnapshotImpl(payload);
}

bool applyContentAliases(WorldMemorySnapshot& snapshot,const ContentIdAliasTable& aliases,std::string* error) {
    for(auto& entity:snapshot.entities) {
        for(auto& ref:entity.contentRefs) {
            auto resolved=aliases.resolve(ref.id,error);
            if(!resolved) return false;
            ref.id=std::move(*resolved);
        }
        std::sort(entity.contentRefs.begin(),entity.contentRefs.end(),[](const auto& a,const auto& b){return a.id<b.id;});
        for(std::size_t i=1;i<entity.contentRefs.size();++i) if(entity.contentRefs[i-1].id==entity.contentRefs[i].id) {
            setError(error,"content alias migration collapsed two entity refs to the same ID");return false;
        }
    }
    for(auto& event:snapshot.events) {
        auto resolved=aliases.resolve(event.typeId,error);
        if(!resolved) return false;
        event.typeId=std::move(*resolved);
    }
    return validateWorldMemorySnapshot(snapshot,error);
}



bool WorldMemoryDiff::logicalChanges() const {
    return !(addedBaselines.empty() && removedBaselines.empty() && changedBaselines.empty() &&
             addedEntities.empty() && removedEntities.empty() && changedEntities.empty() &&
             addedEvents.empty() && removedEvents.empty() && changedEvents.empty());
}

template<class Id, class Record, class GetId>
static void diffStableRecords(const std::vector<Record>& a,const std::vector<Record>& b,GetId getId,
                              std::vector<Id>& added,std::vector<Id>& removed,std::vector<Id>& changed) {
    std::vector<Record> left=a,right=b;
    auto cmp=[&](const Record& x,const Record& y){return getId(x)<getId(y);};
    std::sort(left.begin(),left.end(),cmp); std::sort(right.begin(),right.end(),cmp);
    std::size_t i=0,j=0;
    while(i<left.size() || j<right.size()) {
        if(i==left.size()) { added.push_back(getId(right[j++])); continue; }
        if(j==right.size()) { removed.push_back(getId(left[i++])); continue; }
        const auto li=getId(left[i]), rj=getId(right[j]);
        if(li<rj) { removed.push_back(li); ++i; }
        else if(rj<li) { added.push_back(rj); ++j; }
        else { if(!(left[i]==right[j])) changed.push_back(li); ++i; ++j; }
    }
}

WorldMemoryDiff diffWorldMemorySnapshots(const WorldMemorySnapshot& before,const WorldMemorySnapshot& after) {
    WorldMemoryDiff d{};
    diffStableRecords<SiteId>(before.baselines,after.baselines,[](const SiteBaselineRecord& r){return r.siteId;},d.addedBaselines,d.removedBaselines,d.changedBaselines);
    diffStableRecords<EntityStableId>(before.entities,after.entities,[](const PersistentEntityRecord& r){return r.stableId;},d.addedEntities,d.removedEntities,d.changedEntities);
    diffStableRecords<HistoricalEventId>(before.events,after.events,[](const ChronicleEventRecord& r){return r.eventId;},d.addedEvents,d.removedEvents,d.changedEvents);
    return d;
}

ReclamationAuditResult auditWorldMemoryForReclamation(const WorldMemorySnapshot& snapshot,SiteId siteId) {
    ReclamationAuditResult out{};
    if(siteId==0) { out.errors.push_back("reclamation site ID must be nonzero"); return out; }
    std::string validation;
    if(!validateWorldMemorySnapshot(snapshot,&validation)) { out.errors.push_back(validation); return out; }

    const SiteBaselineRecord* baseline=nullptr;
    for(const auto& b:snapshot.baselines) if(b.siteId==siteId) { baseline=&b; break; }
    if(!baseline) { out.errors.push_back("site has no pinned generator baseline"); return out; }
    out.baseline=*baseline;

    std::unordered_set<EntityStableId> entityIds;
    entityIds.reserve(snapshot.entities.size());
    for(const auto& e:snapshot.entities) entityIds.insert(e.stableId);
    std::unordered_set<HistoricalEventId> eventIds;
    eventIds.reserve(snapshot.events.size());
    for(const auto& e:snapshot.events) eventIds.insert(e.eventId);

    for(const auto& e:snapshot.entities) if(e.location.siteId==siteId) {
        out.siteEntities.push_back(e.stableId);
        for(const auto ref:e.relationshipRefs) if(!entityIds.contains(ref))
            out.warnings.push_back("entity "+std::to_string(e.stableId)+" references nonresident persistent entity "+std::to_string(ref));
        for(const auto ref:e.historyRefs) if(!eventIds.contains(ref))
            out.errors.push_back("entity "+std::to_string(e.stableId)+" references missing Chronicle event "+std::to_string(ref));
    }
    for(const auto& ev:snapshot.events) if(ev.location.siteId==siteId) {
        out.siteEvents.push_back(ev.eventId);
        for(const auto& p:ev.participants) if(!entityIds.contains(p.stableId))
            out.warnings.push_back("Chronicle event "+std::to_string(ev.eventId)+" references nonresident participant "+std::to_string(p.stableId));
        for(const auto ref:ev.causeRefs) if(!eventIds.contains(ref))
            out.errors.push_back("Chronicle event "+std::to_string(ev.eventId)+" references missing cause "+std::to_string(ref));
    }
    std::sort(out.siteEntities.begin(),out.siteEntities.end());
    std::sort(out.siteEvents.begin(),out.siteEvents.end());
    out.valid=out.errors.empty();
    return out;
}

WorldMemoryStore::WorldMemoryStore(std::filesystem::path path):path_(std::move(path)) {}

bool WorldMemoryStore::save(const WorldMemorySnapshot& snapshot,std::string* error) const {
    std::string validation;
    if(!validateWorldMemorySnapshot(snapshot,&validation)) {setError(error,validation);return false;}
    const auto payload=serializeWorldMemorySnapshot(snapshot);
    if(payload.empty()) {setError(error,"world-memory serialization failed");return false;}
    return writeCheckedAtomicGeneration(path_,payload,error);
}

WorldMemoryLoadResult WorldMemoryStore::load(std::optional<std::uint64_t> expectedSaveGeneration) const {
    auto checked=readCheckedAtomicGeneration(path_,false);
    if(!checked.loaded) return {false,checked.usedPreviousGeneration,checked.error,{}};
    auto parsed=parseWorldMemorySnapshot(checked.data);
    parsed.usedPreviousGeneration=checked.usedPreviousGeneration;
    if(!parsed.loaded) return parsed;
    if(!expectedSaveGeneration || parsed.snapshot.saveGeneration==*expectedSaveGeneration) return parsed;

    // If current is valid but belongs to a newer whole-save generation, inspect
    // the retained previous snapshot for the manifest-compatible generation.
    auto previous=path_; previous += ".prev";
    const auto previousChecked=readCheckedAtomicGeneration(previous,false);
    if(previousChecked.loaded) {
        auto fallback=parseWorldMemorySnapshot(previousChecked.data);
        if(fallback.loaded && fallback.snapshot.saveGeneration==*expectedSaveGeneration) {
            fallback.usedPreviousGeneration=true;
            fallback.error="current world-memory generation did not match manifest; recovered previous generation";
            return fallback;
        }
    }
    parsed.loaded=false;
    parsed.error="world-memory save-generation mismatch";
    return parsed;
}


bool WorldMemoryStore::compact(std::optional<std::uint64_t> expectedSaveGeneration,std::string* error) const {
    const auto loaded=load(expectedSaveGeneration);
    if(!loaded.loaded) { setError(error,loaded.error.empty()?"world-memory compaction load failed":loaded.error); return false; }
    const auto canonical=serializeWorldMemorySnapshot(loaded.snapshot);
    if(canonical.empty()) { setError(error,"world-memory compaction serialization failed"); return false; }
    return writeCheckedAtomicGeneration(path_,canonical,error);
}

} // namespace elysium
