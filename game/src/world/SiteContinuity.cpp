// Intended function: imported world implementation for SiteContinuity; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/SiteContinuity.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_set>

namespace elysium {
namespace {

constexpr std::uint64_t kRemoteIdentityLabel = 0x5245544952454944ULL; // RETIREID
constexpr std::uint64_t kHistoryEventLabel = 0x4849535445564E54ULL;    // HISTEVNT
constexpr std::uint64_t kThreatLabel = 0x5448524541543031ULL;          // THREAT01
constexpr std::uint64_t kThreatPickLabel = 0x5448525049434B31ULL;      // THRPICK1

void setError(std::string* error, std::string message) {
    if(error) *error=std::move(message);
}

bool validAddress(const SurfaceCellAddress& a) {
    const int f=static_cast<int>(a.face);
    return f>=static_cast<int>(CubeFace::PositiveX) && f<=static_cast<int>(CubeFace::NegativeZ) &&
           a.radial>=0;
}

bool addressLess(const SurfaceCellAddress& a,const SurfaceCellAddress& b) {
    return std::tie(a.face,a.u,a.v,a.radial)<std::tie(b.face,b.u,b.v,b.radial);
}

bool validBlock(BlockType type) {
    const int value=static_cast<int>(type);
    return value>=0 && value<kBlockTypeCount;
}

std::uint64_t allocateRemoteStableId(RetiredSiteRecord& site) {
    for(;;) {
        const std::uint64_t serial=site.nextRemoteIdentitySerial++;
        std::uint64_t id=mix64(site.siteId ^ kRemoteIdentityLabel ^ mix64(serial));
        if(id==0) id=1;
        const bool figureCollision=std::any_of(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& x){return x.stableId==id;});
        const bool institutionCollision=std::any_of(site.institutions.begin(),site.institutions.end(),[&](const auto& x){return x.stableId==id;});
        if(!figureCollision && !institutionCollision) return id;
    }
}

std::uint64_t allocateEventId(RetiredSiteRecord& site) {
    for(;;) {
        const std::uint64_t serial=site.nextEventSerial++;
        std::uint64_t id=mix64(site.siteId ^ kHistoryEventLabel ^ mix64(serial));
        if(id==0) id=1;
        if(std::none_of(site.history.begin(),site.history.end(),[&](const auto& e){return e.eventId==id;})) return id;
    }
}

std::uint64_t appendEvent(RetiredSiteRecord& site,
                          ContinuityEventType type,
                          std::uint64_t subject=0,
                          std::uint64_t secondary=0,
                          std::int64_t amount=0,
                          std::uint64_t cause=0) {
    ContinuityHistoryEvent event{};
    event.eventId=allocateEventId(site);
    event.day=site.strategicDay;
    event.type=type;
    event.subjectStableId=subject;
    event.secondaryStableId=secondary;
    event.amount=amount;
    event.causeEventId=cause;
    site.history.push_back(event);
    return event.eventId;
}

void appendHistoryRef(std::vector<std::uint64_t>& refs,std::uint64_t eventId) {
    if(eventId==0) return;
    if(std::find(refs.begin(),refs.end(),eventId)==refs.end()) refs.push_back(eventId);
}

std::uint64_t latestEventSerialFloor(const std::vector<ContinuityHistoryEvent>& history) {
    // Event IDs are hashes rather than serial encodings. We only need a safe
    // post-load counter floor; history count + 1 is deterministic and collisions
    // are still checked by allocateEventId().
    return static_cast<std::uint64_t>(history.size())+1;
}

using InfraKey=std::pair<int,std::uint64_t>;

InfraKey infraKey(InfrastructureRecordKind kind,std::uint64_t id) {
    return {static_cast<int>(kind),id};
}

InfraKey infraKey(const InfrastructureJournalRecord& record) {
    return infraKey(record.kind,record.stableId);
}

bool recordIsUpsert(const InfrastructureJournalRecord& record) {
    return record.op==InfrastructureJournalOp::Upsert && record.payload.has_value();
}

bool equalInfrastructureRecord(const InfrastructureJournalRecord& a,const InfrastructureJournalRecord& b) {
    return serializeInfrastructureJournalRecord(a)==serializeInfrastructureJournalRecord(b);
}

bool chunkAddressEqual(const PlanetChunkAddress& a,const PlanetChunkAddress& b) {
    return a.face==b.face && a.u==b.u && a.v==b.v && a.radial==b.radial;
}

bool validChunkAddress(const PlanetChunkAddress& a) {
    const int f=static_cast<int>(a.face);
    return f>=static_cast<int>(CubeFace::PositiveX) && f<=static_cast<int>(CubeFace::NegativeZ) && a.radial>=0;
}

bool captureSparseSpatialState(const PlanetSurface& planet,
                               const std::vector<PlanetChunkAddress>& spatialChunks,
                               RetiredSiteRecord& out,
                               std::string* error) {
    out.macroCells.clear();
    out.microCells.clear();

    for(const auto& chunk:spatialChunks) {
        if(!validChunkAddress(chunk)) {
            setError(error,"retirement spatial scope contains invalid chunk address");
            return false;
        }
    }
    for(std::size_t i=0;i<spatialChunks.size();++i) {
        for(std::size_t j=i+1;j<spatialChunks.size();++j) {
            if(chunkAddressEqual(spatialChunks[i],spatialChunks[j])) {
                setError(error,"retirement spatial scope contains duplicate chunk address");
                return false;
            }
        }
    }

    const auto ownsChunk=[&](const PlanetChunkAddress& address) {
        if(spatialChunks.empty()) return true;
        return std::any_of(spatialChunks.begin(),spatialChunks.end(),[&](const auto& owned){
            return chunkAddressEqual(owned,address);
        });
    };

    for(const auto& [_,journal]:planet.journals()) {
        if(!ownsChunk(journal.address)) continue;
        for(const auto& [flat,type]:journal.macroEdits) {
            const auto address=planet.cellFromFlatIndex(flat);
            if(!validAddress(address) || !validBlock(type)) {
                setError(error,"invalid macro edit while retiring site");
                return false;
            }
            out.macroCells.push_back({address,type,journal.placedMarkers.contains(flat)});
        }
        // A placed marker can exist on a refined cell whose macro material is
        // otherwise baseline, so capture those addresses even without a macro
        // override.
        for(const int flat:journal.placedMarkers) {
            const auto address=planet.cellFromFlatIndex(flat);
            const bool already=std::any_of(out.macroCells.begin(),out.macroCells.end(),[&](const auto& x){return x.address==address;});
            if(!already) out.macroCells.push_back({address,planet.get(address),true});
        }
        for(const auto& [flat,brick]:journal.microBricks) {
            const auto address=planet.cellFromFlatIndex(flat);
            for(const auto& [microIndex,type]:brick.overrides()) {
                if(static_cast<int>(microIndex)>=MicroBrick::CellCount || !validBlock(type)) {
                    setError(error,"invalid micro edit while retiring site");
                    return false;
                }
                out.microCells.push_back({address,microIndex,type});
            }
        }
    }

    std::sort(out.macroCells.begin(),out.macroCells.end(),[](const auto& a,const auto& b){
        return addressLess(a.address,b.address);
    });
    std::sort(out.microCells.begin(),out.microCells.end(),[](const auto& a,const auto& b){
        if(a.address==b.address) return a.microIndex<b.microIndex;
        return addressLess(a.address,b.address);
    });
    return true;
}

bool finalInfrastructure(const RetiredSiteRecord& site,
                         std::vector<InfrastructureJournalRecord>& out,
                         std::string* error) {
    if(!compactInfrastructureJournal(site.infrastructureJournal,out,InfrastructureCompactionPolicy::KeepTombstones,error)) return false;
    if(!validateInfrastructureSnapshotClosure(out,error)) return false;
    return true;
}

int countLocalAliveFigures(const RetiredSiteRecord& site) {
    return static_cast<int>(std::count_if(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& figure){
        return figure.alive && figure.currentSiteId==site.siteId;
    }));
}

void recomputePassiveCondition(RetiredSiteRecord& site) {
    if(site.condition==RetiredSiteCondition::Ruined || site.condition==RetiredSiteCondition::Transformed) return;
    if(site.anonymousPopulation<=0 && countLocalAliveFigures(site)==0) {
        site.condition=RetiredSiteCondition::Abandoned;
        return;
    }
    if(site.contamination>=1.0f) {
        site.condition=RetiredSiteCondition::Contaminated;
        return;
    }
    if(site.threatDamageCount>0) site.condition=RetiredSiteCondition::Damaged;
    else site.condition=RetiredSiteCondition::Occupied;
}

double deterministicUnit(std::uint64_t siteId,std::uint64_t day,std::uint64_t label) {
    const std::uint64_t h=mix64(siteId ^ label ^ mix64(day));
    return static_cast<double>(h>>11U) * (1.0/9007199254740992.0); // 53 bits / 2^53
}

bool validateHistoryRefs(const RetiredSiteRecord& site,std::string* error) {
    std::unordered_set<std::uint64_t> ids;
    ids.reserve(site.history.size()*2+1);
    for(const auto& event:site.history) {
        if(event.eventId==0) { setError(error,"history event has zero StableId"); return false; }
        if(!ids.insert(event.eventId).second) { setError(error,"duplicate history event StableId"); return false; }
        if(event.day>site.strategicDay) { setError(error,"history event occurs after strategic clock"); return false; }
        const int type=static_cast<int>(event.type);
        if(type<0 || type>static_cast<int>(ContinuityEventType::DirectOperativeVisit)) {
            setError(error,"history event type is invalid");
            return false;
        }
    }
    auto check=[&](const std::vector<std::uint64_t>& refs,const char* label) {
        for(const auto id:refs) if(id==0 || !ids.contains(id)) {
            setError(error,std::string(label)+" contains unresolved history reference");
            return false;
        }
        return true;
    };
    for(const auto& figure:site.namedFigures) if(!check(figure.historyRefs,"figure")) return false;
    for(const auto& institution:site.institutions) if(!check(institution.historyRefs,"institution")) return false;
    for(const auto& artifact:site.artifacts) if(!check(artifact.historyRefs,"artifact")) return false;
    for(const auto& event:site.history) if(event.causeEventId!=0 && !ids.contains(event.causeEventId)) {
        setError(error,"history event contains unresolved cause reference");
        return false;
    }
    return true;
}

bool parseBoolInt(int value,bool& out) {
    if(value==0) { out=false; return true; }
    if(value==1) { out=true; return true; }
    return false;
}

std::string hexEncode(std::string_view bytes) {
    static constexpr char digits[]="0123456789ABCDEF";
    std::string out;
    out.reserve(bytes.size()*2);
    for(unsigned char c:bytes) {
        out.push_back(digits[c>>4U]);
        out.push_back(digits[c&0x0FU]);
    }
    return out;
}

std::optional<std::string> hexDecode(std::string_view hex) {
    if(hex.size()%2!=0) return std::nullopt;
    auto nibble=[](char c)->int {
        if(c>='0'&&c<='9') return c-'0';
        if(c>='a'&&c<='f') return c-'a'+10;
        if(c>='A'&&c<='F') return c-'A'+10;
        return -1;
    };
    std::string out;
    out.reserve(hex.size()/2);
    for(std::size_t i=0;i<hex.size();i+=2) {
        const int hi=nibble(hex[i]);
        const int lo=nibble(hex[i+1]);
        if(hi<0 || lo<0) return std::nullopt;
        out.push_back(static_cast<char>((hi<<4)|lo));
    }
    return out;
}

bool readTag(std::istream& in,std::string_view expected,std::string* error) {
    std::string tag;
    if(!(in>>tag) || tag!=expected) {
        setError(error,"expected continuity tag "+std::string(expected));
        return false;
    }
    return true;
}

template<class T>
void sortUniqueIds(std::vector<T>& values) {
    std::sort(values.begin(),values.end(),[](const T& a,const T& b){return a.stableId<b.stableId;});
}

} // namespace

const char* retiredSiteConditionName(RetiredSiteCondition condition) {
    switch(condition) {
        case RetiredSiteCondition::Occupied: return "Occupied";
        case RetiredSiteCondition::Damaged: return "Damaged";
        case RetiredSiteCondition::Contaminated: return "Contaminated";
        case RetiredSiteCondition::Abandoned: return "Abandoned";
        case RetiredSiteCondition::Ruined: return "Ruined";
        case RetiredSiteCondition::Transformed: return "Transformed";
    }
    return "Unknown";
}

const char* continuityEventTypeName(ContinuityEventType type) {
    switch(type) {
        case ContinuityEventType::FortressRetired: return "FortressRetired";
        case ContinuityEventType::PopulationBirths: return "PopulationBirths";
        case ContinuityEventType::PopulationDeaths: return "PopulationDeaths";
        case ContinuityEventType::MigrationIn: return "MigrationIn";
        case ContinuityEventType::MigrationOut: return "MigrationOut";
        case ContinuityEventType::Production: return "Production";
        case ContinuityEventType::OwnershipChanged: return "OwnershipChanged";
        case ContinuityEventType::ThreatIncident: return "ThreatIncident";
        case ContinuityEventType::InfrastructureDestroyed: return "InfrastructureDestroyed";
        case ContinuityEventType::FigureBorn: return "FigureBorn";
        case ContinuityEventType::FigureDied: return "FigureDied";
        case ContinuityEventType::ArtifactMoved: return "ArtifactMoved";
        case ContinuityEventType::InstitutionChanged: return "InstitutionChanged";
        case ContinuityEventType::SiteConditionChanged: return "SiteConditionChanged";
        case ContinuityEventType::SiteReclaimed: return "SiteReclaimed";
        case ContinuityEventType::DirectOperativeVisit: return "DirectOperativeVisit";
    }
    return "Unknown";
}

bool SiteContinuitySystem::retire(const PlanetSurface& planet,
                                  const SurfaceInfrastructure& infrastructure,
                                  const ActiveSiteContinuityInput& input,
                                  RetiredSiteRecord& out,
                                  std::string* error) {
    if(input.siteId==0) { setError(error,"retirement requires nonzero SiteId"); return false; }
    if(input.anonymousPopulation<0 || input.productionStock<0 || !std::isfinite(input.contamination) || input.contamination<0.0f) {
        setError(error,"retirement input contains invalid strategic state"); return false;
    }

    RetiredSiteRecord candidate{};
    candidate.siteId=input.siteId;
    candidate.planetSeed=planet.seed();
    candidate.generatorVersion=PlanetSurface::GeneratorVersion;
    candidate.generatorFingerprint=PlanetSurface::GeneratorFingerprint;
    candidate.planetClass=planet.planetClass();
    candidate.referenceRadius=planet.referenceRadius();
    candidate.retiredDay=input.currentDay;
    candidate.strategicDay=input.currentDay;
    candidate.ownerOrganizationId=input.ownerOrganizationId;
    candidate.claimBeaconStableId=input.claimBeaconStableId;
    candidate.anonymousPopulation=input.anonymousPopulation;
    candidate.productionStock=input.productionStock;
    candidate.contamination=input.contamination;
    candidate.namedFigures=input.namedFigures;
    candidate.institutions=input.institutions;
    candidate.artifacts=input.artifacts;
    candidate.history=input.priorHistory;
    candidate.nextEventSerial=latestEventSerialFloor(candidate.history);

    for(auto& figure:candidate.namedFigures) if(figure.currentSiteId==0) figure.currentSiteId=candidate.siteId;
    for(auto& institution:candidate.institutions) if(institution.siteId==0) institution.siteId=candidate.siteId;
    for(auto& artifact:candidate.artifacts) {
        if(artifact.locationKind==ArtifactLocationKind::AtSite && artifact.siteId==0) artifact.siteId=candidate.siteId;
    }

    if(!captureSparseSpatialState(planet,input.spatialChunks,candidate,error)) return false;
    if(!captureInfrastructureUpserts(infrastructure,candidate.infrastructureJournal,error)) return false;
    if(!validateInfrastructureSnapshotClosure(candidate.infrastructureJournal,error)) return false;

    const auto eventId=appendEvent(candidate,ContinuityEventType::FortressRetired,candidate.siteId,candidate.ownerOrganizationId,
                                   candidate.anonymousPopulation);
    for(auto& figure:candidate.namedFigures) if(figure.currentSiteId==candidate.siteId) appendHistoryRef(figure.historyRefs,eventId);
    candidate.condition=candidate.anonymousPopulation==0 && countLocalAliveFigures(candidate)==0
                        ? RetiredSiteCondition::Abandoned
                        : (candidate.contamination>=1.0f?RetiredSiteCondition::Contaminated:RetiredSiteCondition::Occupied);

    if(!validate(candidate,error)) return false;
    out=std::move(candidate);
    return true;
}

bool SiteContinuitySystem::advance(RetiredSiteRecord& site,
                                   std::uint32_t days,
                                   const StrategicAdvanceInputs& inputs,
                                   std::string* error) {
    if(!validate(site,error)) return false;
    if(!std::isfinite(inputs.birthsPerThousandPerDay) || inputs.birthsPerThousandPerDay<0.0 ||
       !std::isfinite(inputs.deathsPerThousandPerDay) || inputs.deathsPerThousandPerDay<0.0 ||
       !std::isfinite(inputs.migrationPeoplePerDay) ||
       !std::isfinite(inputs.productionPerPersonPerDay) || inputs.productionPerPersonPerDay<0.0 ||
       !std::isfinite(inputs.threatChancePerDay) || inputs.threatChancePerDay<0.0 || inputs.threatChancePerDay>1.0 ||
       inputs.maxPopulationLossPerThreat<0 || !std::isfinite(inputs.contaminationPerThreat) || inputs.contaminationPerThreat<0.0f ||
       !std::isfinite(inputs.ownershipPressurePerDay) || inputs.ownershipPressurePerDay<0.0) {
        setError(error,"strategic advance inputs are invalid");
        return false;
    }

    for(std::uint32_t step=0;step<days;++step) {
        ++site.strategicDay;
        for(auto& figure:site.namedFigures) if(figure.alive && figure.currentSiteId==site.siteId && figure.ageDays<std::numeric_limits<std::uint32_t>::max()) ++figure.ageDays;

        const int populationBefore=site.anonymousPopulation;
        site.birthAccumulator += static_cast<double>(std::max(0,populationBefore))*inputs.birthsPerThousandPerDay/1000.0;
        const int births=static_cast<int>(std::floor(site.birthAccumulator));
        if(births>0) {
            site.birthAccumulator-=births;
            site.anonymousPopulation+=births;
            appendEvent(site,ContinuityEventType::PopulationBirths,site.siteId,0,births);
        }

        site.deathAccumulator += static_cast<double>(std::max(0,site.anonymousPopulation))*inputs.deathsPerThousandPerDay/1000.0;
        int deaths=static_cast<int>(std::floor(site.deathAccumulator));
        deaths=std::min(deaths,site.anonymousPopulation);
        if(deaths>0) {
            site.deathAccumulator-=deaths;
            site.anonymousPopulation-=deaths;
            appendEvent(site,ContinuityEventType::PopulationDeaths,site.siteId,0,deaths);
        }

        site.migrationAccumulator+=inputs.migrationPeoplePerDay;
        if(site.migrationAccumulator>=1.0) {
            const int arrivals=static_cast<int>(std::floor(site.migrationAccumulator));
            site.migrationAccumulator-=arrivals;
            site.anonymousPopulation+=arrivals;
            appendEvent(site,ContinuityEventType::MigrationIn,site.siteId,0,arrivals);
        } else if(site.migrationAccumulator<=-1.0) {
            int departures=static_cast<int>(std::floor(-site.migrationAccumulator));
            departures=std::min(departures,site.anonymousPopulation);
            if(departures>0) {
                site.migrationAccumulator+=departures;
                site.anonymousPopulation-=departures;
                appendEvent(site,ContinuityEventType::MigrationOut,site.siteId,0,departures);
            }
        }

        site.productionAccumulator += static_cast<double>(std::max(0,site.anonymousPopulation))*inputs.productionPerPersonPerDay;
        const auto produced=static_cast<std::int64_t>(std::floor(site.productionAccumulator));
        if(produced>0) {
            site.productionAccumulator-=static_cast<double>(produced);
            site.productionStock+=produced;
            appendEvent(site,ContinuityEventType::Production,site.siteId,0,produced);
        }

        if(inputs.occupyingOrganizationId!=0 && inputs.occupyingOrganizationId!=site.ownerOrganizationId) {
            site.ownershipPressureAccumulator+=inputs.ownershipPressurePerDay;
            if(site.ownershipPressureAccumulator>=1.0) {
                const auto oldOwner=site.ownerOrganizationId;
                site.ownerOrganizationId=inputs.occupyingOrganizationId;
                site.ownershipPressureAccumulator=std::fmod(site.ownershipPressureAccumulator,1.0);
                appendEvent(site,ContinuityEventType::OwnershipChanged,oldOwner,site.ownerOrganizationId,0);
            }
        } else {
            site.ownershipPressureAccumulator=0.0;
        }

        if(inputs.threatChancePerDay>0.0 && deterministicUnit(site.siteId,site.strategicDay,kThreatLabel)<inputs.threatChancePerDay) {
            int losses=0;
            if(site.anonymousPopulation>0 && inputs.maxPopulationLossPerThreat>0) {
                const auto h=mix64(site.siteId ^ kThreatPickLabel ^ mix64(site.strategicDay));
                losses=1+static_cast<int>(h%static_cast<std::uint64_t>(inputs.maxPopulationLossPerThreat));
                losses=std::min(losses,site.anonymousPopulation);
                site.anonymousPopulation-=losses;
            }
            site.contamination=std::max(0.0f,site.contamination+inputs.contaminationPerThreat);
            ++site.threatDamageCount;
            const auto threatEvent=appendEvent(site,ContinuityEventType::ThreatIncident,site.siteId,0,losses);
            if(losses>0) appendEvent(site,ContinuityEventType::PopulationDeaths,site.siteId,0,losses,threatEvent);

            std::vector<InfrastructureJournalRecord> final;
            std::string infraError;
            if(finalInfrastructure(site,final,&infraError)) {
                std::vector<InfrastructureJournalRecord> candidates;
                for(const auto& record:final) if(recordIsUpsert(record)) candidates.push_back(record);
                if(!candidates.empty()) {
                    std::sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b){return infraKey(a)<infraKey(b);});
                    const auto h=mix64(site.siteId ^ kThreatPickLabel ^ mix64(site.strategicDay+0x9e37ULL));
                    const auto& target=candidates[static_cast<std::size_t>(h%candidates.size())];
                    if(!destroyInfrastructure(site,target.kind,target.stableId,&infraError)) {
                        setError(error,"strategic threat could not reconcile infrastructure: "+infraError);
                        return false;
                    }
                }
            }

            if(site.threatDamageCount>=4 && site.anonymousPopulation==0 && countLocalAliveFigures(site)==0) {
                site.condition=RetiredSiteCondition::Ruined;
            }
        }

        const auto before=site.condition;
        recomputePassiveCondition(site);
        if(site.condition!=before) appendEvent(site,ContinuityEventType::SiteConditionChanged,site.siteId,0,static_cast<std::int64_t>(site.condition));
    }

    return validate(site,error);
}

std::uint64_t SiteContinuitySystem::recordNamedDescendantBirth(RetiredSiteRecord& site,
                                                                std::uint64_t parentAStableId,
                                                                std::uint64_t parentBStableId,
                                                                std::uint64_t organizationId,
                                                                std::string* error) {
    if(!validate(site,error)) return 0;
    const auto findParent=[&](std::uint64_t id)->const NamedFigureRecord* {
        if(id==0) return nullptr;
        const auto it=std::find_if(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& f){return f.stableId==id;});
        return it==site.namedFigures.end()?nullptr:&*it;
    };
    const auto* parentA=findParent(parentAStableId);
    const auto* parentB=findParent(parentBStableId);
    if(!parentA) { setError(error,"named descendant requires an existing parent A StableId"); return 0; }
    if(parentBStableId!=0 && !parentB) { setError(error,"named descendant parent B StableId is unresolved"); return 0; }
    const auto inheritedOrganizationId=parentA->organizationId;

    NamedFigureRecord child{};
    child.stableId=allocateRemoteStableId(site);
    child.currentSiteId=site.siteId;
    child.organizationId=organizationId?organizationId:inheritedOrganizationId;
    child.parentAStableId=parentAStableId;
    child.parentBStableId=parentBStableId;
    child.ageDays=0;
    child.alive=true;
    const auto eventId=appendEvent(site,ContinuityEventType::FigureBorn,child.stableId,parentAStableId,0);
    child.historyRefs.push_back(eventId);
    site.namedFigures.push_back(child);
    auto mutableA=std::find_if(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& f){return f.stableId==parentAStableId;});
    if(mutableA!=site.namedFigures.end()) appendHistoryRef(mutableA->historyRefs,eventId);
    if(parentBStableId!=0) {
        auto it=std::find_if(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& f){return f.stableId==parentBStableId;});
        if(it!=site.namedFigures.end()) appendHistoryRef(it->historyRefs,eventId);
    }
    std::sort(site.namedFigures.begin(),site.namedFigures.end(),[](const auto& a,const auto& b){return a.stableId<b.stableId;});
    if(!validate(site,error)) return 0;
    return child.stableId;
}

bool SiteContinuitySystem::recordFigureDeath(RetiredSiteRecord& site,
                                              std::uint64_t figureStableId,
                                              std::string* error) {
    auto it=std::find_if(site.namedFigures.begin(),site.namedFigures.end(),[&](const auto& f){return f.stableId==figureStableId;});
    if(it==site.namedFigures.end()) { setError(error,"figure death StableId not found"); return false; }
    if(!it->alive) return true;
    it->alive=false;
    const auto eventId=appendEvent(site,ContinuityEventType::FigureDied,figureStableId,it->currentSiteId,0);
    appendHistoryRef(it->historyRefs,eventId);
    recomputePassiveCondition(site);
    return validate(site,error);
}

bool SiteContinuitySystem::moveArtifact(RetiredSiteRecord& site,
                                        std::uint64_t artifactId,
                                        ArtifactLocationKind location,
                                        std::uint64_t siteId,
                                        std::uint64_t holderStableId,
                                        std::optional<SurfaceCellAddress> anchor,
                                        std::string* error) {
    auto it=std::find_if(site.artifacts.begin(),site.artifacts.end(),[&](const auto& a){return a.artifactId==artifactId;});
    if(it==site.artifacts.end()) { setError(error,"artifact StableId not found"); return false; }
    if(location==ArtifactLocationKind::AtSite && siteId==0) { setError(error,"artifact AtSite location requires nonzero SiteId"); return false; }
    if(location==ArtifactLocationKind::HeldByFigure && holderStableId==0) { setError(error,"artifact HeldByFigure location requires holder StableId"); return false; }
    if(anchor && !validAddress(*anchor)) { setError(error,"artifact move contains invalid anchor address"); return false; }
    it->locationKind=location;
    it->siteId=siteId;
    it->holderStableId=holderStableId;
    it->anchor=anchor;
    const auto eventId=appendEvent(site,ContinuityEventType::ArtifactMoved,artifactId,holderStableId,static_cast<std::int64_t>(location));
    appendHistoryRef(it->historyRefs,eventId);
    return validate(site,error);
}

bool SiteContinuitySystem::setInstitutionActive(RetiredSiteRecord& site,
                                                std::uint64_t institutionStableId,
                                                bool active,
                                                std::string* error) {
    auto it=std::find_if(site.institutions.begin(),site.institutions.end(),[&](const auto& x){return x.stableId==institutionStableId;});
    if(it==site.institutions.end()) { setError(error,"institution StableId not found"); return false; }
    if(it->active==active) return true;
    it->active=active;
    const auto eventId=appendEvent(site,ContinuityEventType::InstitutionChanged,institutionStableId,it->organizationId,active?1:0);
    appendHistoryRef(it->historyRefs,eventId);
    return validate(site,error);
}

bool SiteContinuitySystem::setCondition(RetiredSiteRecord& site,
                                        RetiredSiteCondition condition,
                                        std::string* error) {
    const int raw=static_cast<int>(condition);
    if(raw<0 || raw>static_cast<int>(RetiredSiteCondition::Transformed)) { setError(error,"invalid retired site condition"); return false; }
    if(site.condition==condition) return true;
    site.condition=condition;
    appendEvent(site,ContinuityEventType::SiteConditionChanged,site.siteId,0,raw);
    return validate(site,error);
}

bool SiteContinuitySystem::destroyInfrastructure(RetiredSiteRecord& site,
                                                 InfrastructureRecordKind kind,
                                                 std::uint64_t stableId,
                                                 std::string* error) {
    if(stableId==0) { setError(error,"cannot destroy infrastructure StableId zero"); return false; }
    std::vector<InfrastructureJournalRecord> final;
    if(!finalInfrastructure(site,final,error)) return false;

    std::map<InfraKey,InfrastructureJournalRecord> upserts;
    for(const auto& record:final) if(recordIsUpsert(record)) upserts.emplace(infraKey(record),record);
    const InfraKey root=infraKey(kind,stableId);
    if(!upserts.contains(root)) {
        // Idempotent if final state is already a tombstone for this key.
        const auto it=std::find_if(final.begin(),final.end(),[&](const auto& r){return infraKey(r)==root && r.op==InfrastructureJournalOp::Tombstone;});
        if(it!=final.end()) return true;
        setError(error,"infrastructure destruction target StableId not found");
        return false;
    }

    std::set<InfraKey> removal;
    removal.insert(root);
    bool changed=true;
    while(changed) {
        changed=false;
        for(const auto& [key,record]:upserts) {
            if(removal.contains(key)) continue;
            for(const auto& dep:record.dependencies) {
                if(removal.contains(infraKey(dep.kind,dep.stableId))) {
                    removal.insert(key);
                    changed=true;
                    break;
                }
            }
        }
    }

    // Append deterministic tombstones. Compaction/application already owns the
    // reverse dependency removal order; sorting here only stabilizes the log.
    std::vector<InfraKey> ordered(removal.begin(),removal.end());
    std::sort(ordered.begin(),ordered.end());
    for(const auto& key:ordered) {
        const auto it=upserts.find(key);
        if(it==upserts.end()) continue;
        site.infrastructureJournal.push_back(makeInfrastructureTombstone(it->second.kind,it->second.stableId,it->second.ownerAddress));
    }
    const auto priorCondition=site.condition;
    if(site.condition!=RetiredSiteCondition::Ruined && site.condition!=RetiredSiteCondition::Transformed)
        site.condition=RetiredSiteCondition::Damaged;
    const auto destructionEvent=appendEvent(site,ContinuityEventType::InfrastructureDestroyed,stableId,static_cast<std::uint64_t>(kind),static_cast<std::int64_t>(removal.size()));
    if(site.condition!=priorCondition)
        appendEvent(site,ContinuityEventType::SiteConditionChanged,site.siteId,0,static_cast<std::int64_t>(site.condition),destructionEvent);
    return validate(site,error);
}

bool SiteContinuitySystem::setRemoteMacroCell(RetiredSiteRecord& site,
                                              SurfaceCellAddress address,
                                              BlockType type,
                                              bool playerPlaced,
                                              std::string* error) {
    if(!validAddress(address) || !validBlock(type)) { setError(error,"remote macro edit is invalid"); return false; }
    auto it=std::lower_bound(site.macroCells.begin(),site.macroCells.end(),address,[](const auto& x,const auto& value){return addressLess(x.address,value);});
    if(it!=site.macroCells.end() && it->address==address) {
        it->type=type;
        it->playerPlaced=playerPlaced;
    } else {
        site.macroCells.insert(it,{address,type,playerPlaced});
    }
    // A macro replacement invalidates prior refined state at that cell just as
    // PlanetSurface::set() does in active simulation.
    site.microCells.erase(std::remove_if(site.microCells.begin(),site.microCells.end(),[&](const auto& m){return m.address==address;}),site.microCells.end());
    return validate(site,error);
}

SiteActivationPlan SiteContinuitySystem::activationPlan(const RetiredSiteRecord& site,
                                                        SiteActivationMode mode) {
    SiteActivationPlan out{};
    out.mode=mode;
    out.siteId=site.siteId;
    out.activateAnonymousPopulation=mode==SiteActivationMode::FortressReclaim;
    out.anonymousPopulationToPromote=out.activateAnonymousPopulation?site.anonymousPopulation:0;
    for(const auto& figure:site.namedFigures) if(figure.alive && figure.currentSiteId==site.siteId) out.namedFigureStableIds.push_back(figure.stableId);
    for(const auto& artifact:site.artifacts) {
        const bool localSite=artifact.siteId==site.siteId;
        const bool materializable=artifact.locationKind==ArtifactLocationKind::AtSite || artifact.locationKind==ArtifactLocationKind::HeldByFigure;
        if(localSite && materializable) out.artifactIds.push_back(artifact.artifactId);
    }
    for(const auto& institution:site.institutions) if(institution.siteId==site.siteId && institution.active) out.institutionStableIds.push_back(institution.stableId);
    std::sort(out.namedFigureStableIds.begin(),out.namedFigureStableIds.end());
    std::sort(out.artifactIds.begin(),out.artifactIds.end());
    std::sort(out.institutionStableIds.begin(),out.institutionStableIds.end());
    return out;
}

bool SiteContinuitySystem::reclaim(RetiredSiteRecord& site,
                                   PlanetSurface& planet,
                                   SurfaceInfrastructure& infrastructure,
                                   SiteActivationMode mode,
                                   SiteActivationPlan* plan,
                                   SiteContinuityAudit* outAudit,
                                   std::string* error) {
    if(!validate(site,error)) return false;
    if(planet.seed()!=site.planetSeed || planet.planetClass()!=site.planetClass ||
       PlanetSurface::GeneratorVersion!=site.generatorVersion ||
       PlanetSurface::GeneratorFingerprint!=site.generatorFingerprint ||
       std::abs(planet.referenceRadius()-site.referenceRadius)>1e-4f) {
        setError(error,"reclamation generator/world identity mismatch");
        return false;
    }

    for(const auto& macro:site.macroCells) {
        planet.set(macro.address,macro.type,false);
        if(macro.playerPlaced && blockProperties(planet.get(macro.address)).solid)
            planet.applySavedPlacedMarker(planet.flatIndex(macro.address));
    }
    for(const auto& micro:site.microCells) planet.setMicroIndex(micro.address,micro.microIndex,micro.type);

    std::vector<InfrastructureJournalRecord> desired;
    if(!finalInfrastructure(site,desired,error)) return false;

    // Capture already-live objects and explicitly resolve idempotent matches or
    // conflicts before mutation. This is the reclaim duplicate-prevention gate.
    std::vector<InfrastructureJournalRecord> live;
    if(!captureInfrastructureUpserts(infrastructure,live,error)) return false;
    std::map<InfraKey,InfrastructureJournalRecord> liveByKey;
    for(const auto& record:live) liveByKey.emplace(infraKey(record),record);

    std::vector<InfrastructureJournalRecord> toApply;
    for(const auto& record:desired) {
        const auto key=infraKey(record);
        const auto lit=liveByKey.find(key);
        if(record.op==InfrastructureJournalOp::Tombstone) {
            if(lit!=liveByKey.end()) toApply.push_back(record);
            continue;
        }
        if(lit==liveByKey.end()) {
            toApply.push_back(record);
            continue;
        }
        if(!equalInfrastructureRecord(record,lit->second)) {
            setError(error,"reclamation conflict for live infrastructure StableId "+std::to_string(record.stableId));
            return false;
        }
    }
    if(!toApply.empty() && !applyInfrastructureJournal(infrastructure,planet,toApply,error)) return false;

    const auto historyType=mode==SiteActivationMode::FortressReclaim?ContinuityEventType::SiteReclaimed:ContinuityEventType::DirectOperativeVisit;
    appendEvent(site,historyType,site.siteId,site.ownerOrganizationId,0);
    if(mode==SiteActivationMode::FortressReclaim && site.condition==RetiredSiteCondition::Abandoned &&
       (site.anonymousPopulation>0 || countLocalAliveFigures(site)>0)) {
        site.condition=RetiredSiteCondition::Occupied;
    }

    const auto builtPlan=activationPlan(site,mode);
    if(plan) *plan=builtPlan;
    const auto checked=audit(site);
    if(outAudit) *outAudit=checked;
    if(!checked.valid) { setError(error,checked.error); return false; }
    return true;
}

SiteContinuityAudit SiteContinuitySystem::audit(const RetiredSiteRecord& site) {
    SiteContinuityAudit out{};
    std::string error;
    if(!validate(site,&error)) { out.error=std::move(error); return out; }
    std::vector<InfrastructureJournalRecord> final;
    if(!finalInfrastructure(site,final,&error)) { out.error=std::move(error); return out; }
    for(const auto& record:final) {
        if(record.op==InfrastructureJournalOp::Tombstone) ++out.infrastructureTombstones;
        else ++out.survivingInfrastructureObjects;
    }
    out.namedFiguresAtSite=countLocalAliveFigures(site);
    out.artifactsAtSite=static_cast<int>(std::count_if(site.artifacts.begin(),site.artifacts.end(),[&](const auto& artifact){
        return artifact.siteId==site.siteId && artifact.locationKind!=ArtifactLocationKind::Lost && artifact.locationKind!=ArtifactLocationKind::Destroyed;
    }));
    out.activeInstitutions=static_cast<int>(std::count_if(site.institutions.begin(),site.institutions.end(),[&](const auto& institution){
        return institution.siteId==site.siteId && institution.active;
    }));
    out.valid=true;
    return out;
}

std::string SiteContinuitySystem::inspect(const RetiredSiteRecord& site) {
    const auto checked=audit(site);
    std::ostringstream out;
    out<<"Retired site "<<site.siteId<<"\n";
    if(!checked.valid) {
        out<<"status: INVALID\nreason: "<<checked.error<<"\n";
        return out.str();
    }
    out<<"status: "<<retiredSiteConditionName(site.condition)<<"\n"
       <<"clock: retired day "<<site.retiredDay<<", strategic day "<<site.strategicDay<<"\n"
       <<"owner organization: "<<site.ownerOrganizationId<<"\n"
       <<"population: "<<site.anonymousPopulation<<" anonymous + "<<checked.namedFiguresAtSite<<" named local\n"
       <<"production stock: "<<site.productionStock<<"\n"
       <<"contamination: "<<site.contamination<<"\n"
       <<"spatial deltas: "<<site.macroCells.size()<<" macro, "<<site.microCells.size()<<" micro\n"
       <<"infrastructure: "<<checked.survivingInfrastructureObjects<<" surviving, "<<checked.infrastructureTombstones<<" tombstoned\n"
       <<"institutions: "<<checked.activeInstitutions<<" active\n"
       <<"artifacts at site: "<<checked.artifactsAtSite<<"\n"
       <<"history events: "<<site.history.size()<<"\n"
       <<"generator: v"<<site.generatorVersion<<" / "<<site.generatorFingerprint<<"\n";
    if(!site.history.empty()) {
        const auto& event=site.history.back();
        out<<"last event: "<<continuityEventTypeName(event.type)<<" on day "<<event.day
           <<" (event "<<event.eventId<<")\n";
    }
    return out.str();
}

bool SiteContinuitySystem::validate(const RetiredSiteRecord& site,std::string* error) {
    if(site.schemaVersion!=RetiredSiteRecord::SchemaVersion) { setError(error,"unsupported retired-site schema version"); return false; }
    if(site.siteId==0 || site.planetSeed==0) { setError(error,"retired site has zero stable world/site identity"); return false; }
    if(site.generatorVersion<=0 || site.generatorFingerprint==0) { setError(error,"retired site lacks generator compatibility metadata"); return false; }
    if(!std::isfinite(site.referenceRadius) || site.referenceRadius<=0.0f || site.strategicDay<site.retiredDay) { setError(error,"retired site clock/world metadata invalid"); return false; }
    const int condition=static_cast<int>(site.condition);
    if(condition<0 || condition>static_cast<int>(RetiredSiteCondition::Transformed)) { setError(error,"retired site condition invalid"); return false; }
    if(site.anonymousPopulation<0 || site.productionStock<0 || !std::isfinite(site.contamination) || site.contamination<0.0f ||
       !std::isfinite(site.birthAccumulator) || site.birthAccumulator<0.0 ||
       !std::isfinite(site.deathAccumulator) || site.deathAccumulator<0.0 ||
       !std::isfinite(site.migrationAccumulator) || !std::isfinite(site.productionAccumulator) || site.productionAccumulator<0.0 ||
       !std::isfinite(site.ownershipPressureAccumulator) || site.ownershipPressureAccumulator<0.0) {
        setError(error,"retired site strategic state invalid"); return false;
    }
    if(site.nextRemoteIdentitySerial==0 || site.nextEventSerial==0) { setError(error,"retired site serial counter wrapped to zero"); return false; }

    std::unordered_set<std::uint64_t> figureIds;
    for(const auto& figure:site.namedFigures) {
        if(figure.stableId==0 || !figureIds.insert(figure.stableId).second) { setError(error,"duplicate/zero named figure StableId"); return false; }
        if(figure.currentSiteId==0) { setError(error,"named figure has zero strategic SiteId"); return false; }
        if(figure.parentAStableId==figure.stableId || figure.parentBStableId==figure.stableId) { setError(error,"named figure is its own parent"); return false; }
    }
    std::unordered_set<std::uint64_t> institutionIds;
    for(const auto& institution:site.institutions) {
        if(institution.stableId==0 || !institutionIds.insert(institution.stableId).second || institution.siteId==0) {
            setError(error,"invalid/duplicate institution StableId"); return false;
        }
        const int kind=static_cast<int>(institution.kind);
        if(kind<0 || kind>static_cast<int>(ContinuityInstitutionKind::Other)) { setError(error,"institution kind invalid"); return false; }
    }
    std::unordered_set<std::uint64_t> artifactIds;
    for(const auto& artifact:site.artifacts) {
        if(artifact.artifactId==0 || !artifactIds.insert(artifact.artifactId).second) { setError(error,"invalid/duplicate ArtifactId"); return false; }
        const int location=static_cast<int>(artifact.locationKind);
        if(location<0 || location>static_cast<int>(ArtifactLocationKind::Destroyed)) { setError(error,"artifact location kind invalid"); return false; }
        if(artifact.locationKind==ArtifactLocationKind::AtSite && artifact.siteId==0) { setError(error,"artifact AtSite location missing SiteId"); return false; }
        if(artifact.locationKind==ArtifactLocationKind::HeldByFigure && artifact.holderStableId==0) { setError(error,"artifact holder location missing StableId"); return false; }
        if(artifact.anchor && !validAddress(*artifact.anchor)) { setError(error,"artifact anchor invalid"); return false; }
    }

    if(!validateHistoryRefs(site,error)) return false;

    SurfaceCellAddress previous{};
    bool havePrevious=false;
    for(const auto& macro:site.macroCells) {
        if(!validAddress(macro.address) || !validBlock(macro.type)) { setError(error,"retired macro spatial record invalid"); return false; }
        if(havePrevious && !addressLess(previous,macro.address)) { setError(error,"retired macro spatial records are not unique/sorted"); return false; }
        previous=macro.address; havePrevious=true;
    }
    for(std::size_t i=0;i<site.microCells.size();++i) {
        const auto& micro=site.microCells[i];
        if(!validAddress(micro.address) || micro.microIndex<0 || micro.microIndex>=MicroBrick::CellCount || !validBlock(micro.type)) {
            setError(error,"retired micro spatial record invalid"); return false;
        }
        if(i>0) {
            const auto& prior=site.microCells[i-1];
            if(prior.address==micro.address) {
                if(prior.microIndex>=micro.microIndex) { setError(error,"retired micro records are not unique/sorted"); return false; }
            } else if(!addressLess(prior.address,micro.address)) {
                setError(error,"retired micro records are not address sorted"); return false;
            }
        }
    }

    std::vector<InfrastructureJournalRecord> compacted;
    if(!compactInfrastructureJournal(site.infrastructureJournal,compacted,InfrastructureCompactionPolicy::KeepTombstones,error)) return false;
    if(!validateInfrastructureSnapshotClosure(compacted,error)) return false;
    return true;
}

std::string SiteContinuitySystem::serialize(const RetiredSiteRecord& site) {
    std::string error;
    if(!validate(site,&error)) return {};
    std::ostringstream out;
    out<<std::setprecision(17);
    out<<"ELYSIUM_RETIRED_SITE "<<site.schemaVersion<<"\n";
    out<<"meta "<<site.siteId<<' '<<site.planetSeed<<' '<<site.generatorVersion<<' '<<site.generatorFingerprint<<' '
       <<static_cast<int>(site.planetClass)<<' '<<site.referenceRadius<<' '<<site.retiredDay<<' '<<site.strategicDay<<"\n";
    out<<"state "<<site.ownerOrganizationId<<' '<<site.claimBeaconStableId<<' '<<static_cast<int>(site.condition)<<' '
       <<site.anonymousPopulation<<' '<<site.productionStock<<' '<<site.contamination<<' '
       <<site.birthAccumulator<<' '<<site.deathAccumulator<<' '<<site.migrationAccumulator<<' '
       <<site.productionAccumulator<<' '<<site.ownershipPressureAccumulator<<' '<<site.threatDamageCount<<' '
       <<site.nextRemoteIdentitySerial<<' '<<site.nextEventSerial<<"\n";

    out<<"history "<<site.history.size()<<"\n";
    for(const auto& event:site.history)
        out<<"event "<<event.eventId<<' '<<event.day<<' '<<static_cast<int>(event.type)<<' '<<event.subjectStableId<<' '
           <<event.secondaryStableId<<' '<<event.amount<<' '<<event.causeEventId<<"\n";

    out<<"figures "<<site.namedFigures.size()<<"\n";
    for(const auto& figure:site.namedFigures) {
        out<<"figure "<<figure.stableId<<' '<<figure.currentSiteId<<' '<<figure.organizationId<<' '
           <<figure.parentAStableId<<' '<<figure.parentBStableId<<' '<<figure.ageDays<<' '<<(figure.alive?1:0)<<' '
           <<figure.historyRefs.size();
        for(const auto id:figure.historyRefs) out<<' '<<id;
        out<<"\n";
    }

    out<<"institutions "<<site.institutions.size()<<"\n";
    for(const auto& institution:site.institutions) {
        out<<"institution "<<institution.stableId<<' '<<institution.siteId<<' '<<institution.organizationId<<' '
           <<static_cast<int>(institution.kind)<<' '<<(institution.active?1:0)<<' '<<institution.historyRefs.size();
        for(const auto id:institution.historyRefs) out<<' '<<id;
        out<<"\n";
    }

    out<<"artifacts "<<site.artifacts.size()<<"\n";
    for(const auto& artifact:site.artifacts) {
        out<<"artifact "<<artifact.artifactId<<' '<<static_cast<int>(artifact.locationKind)<<' '<<artifact.siteId<<' '
           <<artifact.holderStableId<<' '<<artifact.ownerOrganizationId<<' '<<(artifact.anchor?1:0);
        if(artifact.anchor) out<<' '<<static_cast<int>(artifact.anchor->face)<<' '<<artifact.anchor->u<<' '<<artifact.anchor->v<<' '<<artifact.anchor->radial;
        out<<' '<<artifact.historyRefs.size();
        for(const auto id:artifact.historyRefs) out<<' '<<id;
        out<<"\n";
    }

    out<<"macro "<<site.macroCells.size()<<"\n";
    for(const auto& cell:site.macroCells)
        out<<"m "<<static_cast<int>(cell.address.face)<<' '<<cell.address.u<<' '<<cell.address.v<<' '<<cell.address.radial<<' '
           <<static_cast<int>(cell.type)<<' '<<(cell.playerPlaced?1:0)<<"\n";

    out<<"micro "<<site.microCells.size()<<"\n";
    for(const auto& cell:site.microCells)
        out<<"u "<<static_cast<int>(cell.address.face)<<' '<<cell.address.u<<' '<<cell.address.v<<' '<<cell.address.radial<<' '
           <<cell.microIndex<<' '<<static_cast<int>(cell.type)<<"\n";

    out<<"infra "<<site.infrastructureJournal.size()<<"\n";
    for(const auto& record:site.infrastructureJournal) {
        const auto encoded=hexEncode(serializeInfrastructureJournalRecord(record));
        out<<"i "<<encoded<<"\n";
    }
    out<<"END\n";
    return out.str();
}

std::optional<RetiredSiteRecord> SiteContinuitySystem::deserialize(std::string_view text,std::string* error) {
    std::istringstream in{std::string(text)};
    std::string magic;
    std::uint32_t version{};
    if(!(in>>magic>>version) || magic!="ELYSIUM_RETIRED_SITE") { setError(error,"invalid retired-site header"); return std::nullopt; }
    if(version!=RetiredSiteRecord::SchemaVersion) { setError(error,"unsupported retired-site schema version"); return std::nullopt; }

    RetiredSiteRecord site{};
    site.schemaVersion=version;
    int planetClass{},condition{};
    if(!readTag(in,"meta",error) || !(in>>site.siteId>>site.planetSeed>>site.generatorVersion>>site.generatorFingerprint>>planetClass>>site.referenceRadius>>site.retiredDay>>site.strategicDay)) return std::nullopt;
    site.planetClass=static_cast<PlanetClass>(planetClass);
    if(!readTag(in,"state",error) || !(in>>site.ownerOrganizationId>>site.claimBeaconStableId>>condition>>site.anonymousPopulation>>site.productionStock>>site.contamination
                                        >>site.birthAccumulator>>site.deathAccumulator>>site.migrationAccumulator>>site.productionAccumulator>>site.ownershipPressureAccumulator
                                        >>site.threatDamageCount>>site.nextRemoteIdentitySerial>>site.nextEventSerial)) return std::nullopt;
    site.condition=static_cast<RetiredSiteCondition>(condition);

    std::size_t count{};
    if(!readTag(in,"history",error) || !(in>>count) || count>1000000) { setError(error,"invalid retired-site history count"); return std::nullopt; }
    site.history.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        ContinuityHistoryEvent event{}; int type{};
        if(!readTag(in,"event",error) || !(in>>event.eventId>>event.day>>type>>event.subjectStableId>>event.secondaryStableId>>event.amount>>event.causeEventId)) return std::nullopt;
        event.type=static_cast<ContinuityEventType>(type);
        site.history.push_back(event);
    }

    if(!readTag(in,"figures",error) || !(in>>count) || count>1000000) { setError(error,"invalid retired-site figure count"); return std::nullopt; }
    site.namedFigures.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        NamedFigureRecord figure{}; int alive{}; std::size_t refs{};
        if(!readTag(in,"figure",error) || !(in>>figure.stableId>>figure.currentSiteId>>figure.organizationId>>figure.parentAStableId>>figure.parentBStableId>>figure.ageDays>>alive>>refs) || refs>1000000) return std::nullopt;
        if(!parseBoolInt(alive,figure.alive)) { setError(error,"invalid figure alive flag"); return std::nullopt; }
        figure.historyRefs.resize(refs);
        for(auto& id:figure.historyRefs) if(!(in>>id)) { setError(error,"truncated figure history refs"); return std::nullopt; }
        site.namedFigures.push_back(std::move(figure));
    }

    if(!readTag(in,"institutions",error) || !(in>>count) || count>1000000) { setError(error,"invalid retired-site institution count"); return std::nullopt; }
    site.institutions.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        InstitutionContinuityRecord record{}; int kind{},active{}; std::size_t refs{};
        if(!readTag(in,"institution",error) || !(in>>record.stableId>>record.siteId>>record.organizationId>>kind>>active>>refs) || refs>1000000) return std::nullopt;
        record.kind=static_cast<ContinuityInstitutionKind>(kind);
        if(!parseBoolInt(active,record.active)) { setError(error,"invalid institution active flag"); return std::nullopt; }
        record.historyRefs.resize(refs);
        for(auto& id:record.historyRefs) if(!(in>>id)) { setError(error,"truncated institution history refs"); return std::nullopt; }
        site.institutions.push_back(std::move(record));
    }

    if(!readTag(in,"artifacts",error) || !(in>>count) || count>1000000) { setError(error,"invalid retired-site artifact count"); return std::nullopt; }
    site.artifacts.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        ArtifactContinuityRecord record{}; int location{},hasAnchor{}; std::size_t refs{};
        if(!readTag(in,"artifact",error) || !(in>>record.artifactId>>location>>record.siteId>>record.holderStableId>>record.ownerOrganizationId>>hasAnchor)) return std::nullopt;
        record.locationKind=static_cast<ArtifactLocationKind>(location);
        if(hasAnchor==1) {
            SurfaceCellAddress address{}; int face{};
            if(!(in>>face>>address.u>>address.v>>address.radial)) { setError(error,"truncated artifact anchor"); return std::nullopt; }
            address.face=static_cast<CubeFace>(face);
            record.anchor=address;
        } else if(hasAnchor!=0) { setError(error,"invalid artifact anchor flag"); return std::nullopt; }
        if(!(in>>refs) || refs>1000000) { setError(error,"invalid artifact history ref count"); return std::nullopt; }
        record.historyRefs.resize(refs);
        for(auto& id:record.historyRefs) if(!(in>>id)) { setError(error,"truncated artifact history refs"); return std::nullopt; }
        site.artifacts.push_back(std::move(record));
    }

    if(!readTag(in,"macro",error) || !(in>>count) || count>10000000) { setError(error,"invalid retired-site macro count"); return std::nullopt; }
    site.macroCells.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        RetiredMacroCellState cell{}; int face{},type{},placed{};
        if(!readTag(in,"m",error) || !(in>>face>>cell.address.u>>cell.address.v>>cell.address.radial>>type>>placed)) return std::nullopt;
        cell.address.face=static_cast<CubeFace>(face);
        cell.type=static_cast<BlockType>(type);
        if(!parseBoolInt(placed,cell.playerPlaced)) { setError(error,"invalid macro placed flag"); return std::nullopt; }
        site.macroCells.push_back(cell);
    }

    if(!readTag(in,"micro",error) || !(in>>count) || count>20000000) { setError(error,"invalid retired-site micro count"); return std::nullopt; }
    site.microCells.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        RetiredMicroCellState cell{}; int face{},type{};
        if(!readTag(in,"u",error) || !(in>>face>>cell.address.u>>cell.address.v>>cell.address.radial>>cell.microIndex>>type)) return std::nullopt;
        cell.address.face=static_cast<CubeFace>(face);
        cell.type=static_cast<BlockType>(type);
        site.microCells.push_back(cell);
    }

    if(!readTag(in,"infra",error) || !(in>>count) || count>1000000) { setError(error,"invalid retired-site infrastructure count"); return std::nullopt; }
    site.infrastructureJournal.reserve(count);
    for(std::size_t i=0;i<count;++i) {
        std::string encoded;
        if(!readTag(in,"i",error) || !(in>>encoded)) return std::nullopt;
        auto decoded=hexDecode(encoded);
        if(!decoded) { setError(error,"invalid hex infrastructure payload in retired site"); return std::nullopt; }
        auto record=deserializeInfrastructureJournalRecord(*decoded,error);
        if(!record) return std::nullopt;
        site.infrastructureJournal.push_back(std::move(*record));
    }

    if(!readTag(in,"END",error)) return std::nullopt;
    std::string trailing;
    if(in>>trailing) { setError(error,"retired-site payload has trailing data"); return std::nullopt; }
    if(!validate(site,error)) return std::nullopt;
    return site;
}

} // namespace elysium
