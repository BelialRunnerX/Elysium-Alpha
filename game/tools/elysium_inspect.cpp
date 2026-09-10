// Intended function: imported tools implementation for elysium_inspect; preserves the agent-authored subsystem contract for later integration/debugging.
#include "tools/DeveloperObservability.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

using namespace elysium;

namespace {

int parseInt(const char* text,const char* label) {
    try { return std::stoi(text); }
    catch(...) { throw std::runtime_error(std::string("invalid ")+label+": "+text); }
}

std::uint64_t parseU64(const char* text,const char* label) {
    try { return std::stoull(text,nullptr,0); }
    catch(...) { throw std::runtime_error(std::string("invalid ")+label+": "+text); }
}

PlanetClass parsePlanetClass(int value) {
    if(value<0 || value>static_cast<int>(PlanetClass::Anomalous)) throw std::runtime_error("planetClass must be 0..7 (Temperate/Barren/Scorched/Frozen/Toxic/Irradiated/Oceanic/Anomalous)");
    return static_cast<PlanetClass>(value);
}

CubeFace parseFace(int value) {
    if(value<0 || value>=PlanetSurface::FaceCount) throw std::runtime_error("face must be 0..5");
    return static_cast<CubeFace>(value);
}

void usage() {
    std::cout
        <<"elysium_inspect - headless Elysium developer observability\n\n"
        <<"Commands:\n"
        <<"  demo\n"
        <<"      Build a deterministic local fixture and print Why/power/persistence/trace views.\n"
        <<"  chunk-record <root> <planetSlot> <planetSeed> <face 0..5> <cu> <cv> <cr> [saveGeneration]\n"
        <<"      Load a transactional surface chunk sidecar and dump its stable objects/deltas.\n"
        <<"  worldgen-window <seed> <planetClass 0..7> <face 0..5> <u> <v> <width> <height> [maxSamples]\n"
        <<"      Dump a bounded surface-height/material window without allocating planet-wide state.\n"
        <<"  cross-section <seed> <planetClass 0..7> <face 0..5> <fixedV> <uBegin> <width> <radialBegin> <radialEnd> [maxSamples]\n"
        <<"      Dump a bounded vertical material cross-section.\n"
        <<"  mesh <seed> <planetClass 0..7> <face 0..5> <cu> <cv> <cr> [fieldStep]\n"
        <<"      Build/read a CPU mesh packet; omit fieldStep for full detail, pass >=1 for field proxy.\n"
        <<"  snapshot-demo\n"
        <<"      Print representative remote-entity, citizen, relationship, environment, medical, military, Chronicle, GPU and frame-phase views.\n";
}

int demo() {
    PlanetSurface planet(0x43ULL,PlanetClass::Barren);
    SurfaceInfrastructure infrastructure(0x43ULL);
    SurfaceIndustrySystem industry;
    SystemTraceRecorder trace(256);
    // Trace sinks were retired from the authoritative infrastructure/industry
    // contracts. The recorder remains useful for snapshot/format inspection;
    // runtime Why/power/persistence views query the authoritative systems directly.

    const CubeFace face=CubeFace::PositiveZ;
    const int u0=20,v0=20,r0=24;
    for(int r=r0;r<r0+5;++r) for(int v=v0;v<v0+5;++v) for(int u=u0;u<u0+5;++u) {
        const bool shell=u==u0||u==u0+4||v==v0||v==v0+4||r==r0||r==r0+4;
        planet.set({face,u,v,r},shell?BlockType::SteelPlate:BlockType::Air,true);
    }
    const SurfaceCellAddress inside{face,u0+2,v0+2,r0+2};
    const auto atmosphere=infrastructure.place(MachineType::AtmosphereUnit,inside);
    infrastructure.update(planet,0.1f);

    std::cout<<"=== SEALED ROOM / BROWNOUT ===\n";
    std::cout<<DeveloperObservability::toText(DeveloperObservability::why(planet,infrastructure,atmosphere,512))<<'\n';

    const auto generator=infrastructure.place(MachineType::BurnerGenerator,{face,u0+1,v0+1,r0+1});
    if(auto* g=infrastructure.find(generator)) g->fuelSeconds=10.0f;
    infrastructure.update(planet,0.5f);
    industry.update(infrastructure,0.1f);
    planet.set({face,u0+2,v0+2,r0+4},BlockType::Air,true);
    infrastructure.update(planet,0.1f);

    std::cout<<"=== OPEN BREACH ===\n";
    std::cout<<DeveloperObservability::toText(DeveloperObservability::why(planet,infrastructure,atmosphere,8192))<<'\n';
    std::cout<<DeveloperObservability::toText(DeveloperObservability::power(infrastructure))<<'\n';
    std::cout<<DeveloperObservability::toText(DeveloperObservability::persistenceDiff(
        planet,infrastructure,planet.chunkOf(inside)))<<'\n';
    std::cout<<DeveloperObservability::toText(trace);
    return 0;
}

int chunkRecord(int argc,char** argv) {
    if(argc<9 || argc>10) { usage(); return 2; }
    const std::filesystem::path root=argv[2];
    const int planetSlot=parseInt(argv[3],"planetSlot");
    const auto seed=parseU64(argv[4],"planetSeed");
    PlanetChunkAddress address{parseFace(parseInt(argv[5],"face")),parseInt(argv[6],"cu"),parseInt(argv[7],"cv"),parseInt(argv[8],"cr")};
    std::optional<std::uint64_t> generation;
    if(argc==10) generation=parseU64(argv[9],"saveGeneration");

    SurfaceChunkStore store(root,planetSlot,seed,kSurfaceGeneratorVersion,kSurfaceGeneratorFingerprint);
    const auto loaded=store.load(address,generation);
    if(!loaded.loaded) {
        std::cerr<<"load failed: "<<loaded.error<<'\n';
        return 1;
    }
    std::cout<<"previousFallback="<<(loaded.usedPreviousGeneration?"true":"false")<<'\n';
    std::cout<<DeveloperObservability::toText(DeveloperObservability::chunkRecord(loaded.record));
    return 0;
}

int worldgenWindow(int argc,char** argv) {
    if(argc<9 || argc>10) { usage(); return 2; }
    const auto seed=parseU64(argv[2],"seed");
    const auto planetClass=parsePlanetClass(parseInt(argv[3],"planetClass"));
    const auto face=parseFace(parseInt(argv[4],"face"));
    const int u=parseInt(argv[5],"u");
    const int v=parseInt(argv[6],"v");
    const int width=parseInt(argv[7],"width");
    const int height=parseInt(argv[8],"height");
    const std::size_t maxSamples=argc==10?static_cast<std::size_t>(parseU64(argv[9],"maxSamples")):4096U;
    PlanetSurface planet(seed,planetClass);
    std::cout<<DeveloperObservability::toText(DeveloperObservability::worldgenWindow(planet,face,u,v,width,height,maxSamples));
    return 0;
}

int crossSection(int argc,char** argv) {
    if(argc<10 || argc>11) { usage(); return 2; }
    const auto seed=parseU64(argv[2],"seed");
    const auto planetClass=parsePlanetClass(parseInt(argv[3],"planetClass"));
    const auto face=parseFace(parseInt(argv[4],"face"));
    const int fixedV=parseInt(argv[5],"fixedV");
    const int uBegin=parseInt(argv[6],"uBegin");
    const int width=parseInt(argv[7],"width");
    const int radialBegin=parseInt(argv[8],"radialBegin");
    const int radialEnd=parseInt(argv[9],"radialEnd");
    const std::size_t maxSamples=argc==11?static_cast<std::size_t>(parseU64(argv[10],"maxSamples")):8192U;
    PlanetSurface planet(seed,planetClass);
    std::cout<<DeveloperObservability::toText(DeveloperObservability::crossSection(planet,face,fixedV,uBegin,width,radialBegin,radialEnd,maxSamples));
    return 0;
}

int mesh(int argc,char** argv) {
    if(argc<8 || argc>9) { usage(); return 2; }
    const auto seed=parseU64(argv[2],"seed");
    const auto planetClass=parsePlanetClass(parseInt(argv[3],"planetClass"));
    PlanetChunkAddress address{parseFace(parseInt(argv[4],"face")),parseInt(argv[5],"cu"),parseInt(argv[6],"cv"),parseInt(argv[7],"cr")};
    const bool fieldProxy=argc==9;
    const int fieldStep=fieldProxy?parseInt(argv[8],"fieldStep"):4;
    PlanetSurface planet(seed,planetClass);
    std::cout<<DeveloperObservability::toText(DeveloperObservability::mesh(planet,address,fieldProxy,fieldStep));
    return 0;
}

int snapshotDemo() {
    EntityDiagnosticSnapshot entity{};
    entity.stableId=430001;
    entity.loaded=false;
    entity.remote=true;
    entity.kind="historical-person";
    entity.shard="galaxy-history";
    entity.components={{"OfficeHolding","Frontier Administrator"},{"HistorySignificance","named"}};
    std::cout<<DeveloperObservability::toText(DeveloperObservability::entity(entity))<<'\n';

    CitizenDiagnosticSnapshot citizen{};
    citizen.stableId=430001;
    citizen.loaded=false;
    citizen.remote=true;
    citizen.name="Agent43 Fixture Citizen";
    citizen.state="strategic";
    citizen.stress=0.72f;
    citizen.focus=0.58f;
    citizen.influentialNeeds={"sleep","social contact"};
    citizen.influentialMemories={"reactor evacuation"};
    citizen.capabilityLimits={"remote representation: no local path state"};
    std::cout<<DeveloperObservability::toText(DeveloperObservability::citizen(citizen))<<'\n';

    const std::vector<RelationshipEdgeDiagnostic> relationships{
        {430001,430002,"mentor",0.7f,0.8f,0.9f,0.1f,""},
        {430003,430001,"rival",-0.4f,0.2f,0.3f,0.5f,"disputed salvage claim"}
    };
    std::cout<<DeveloperObservability::toText(DeveloperObservability::relationships(430001,relationships))<<'\n';

    EnvironmentalVolumeDiagnosticSnapshot env{};
    env.stableId=430010;
    env.loaded=true;
    env.kind="quarantine-room";
    env.cells=312;
    env.bounded=true;
    env.pressure=0.82f;
    env.oxygen=0.19f;
    env.temperature=296.0f;
    env.contamination=0.35f;
    env.activeSources={"damaged filter","rift residue"};
    std::cout<<DeveloperObservability::toText(DeveloperObservability::environment(env))<<'\n';

    MedicalDiagnosticSnapshot medical{};
    medical.citizenStableId=430001;
    medical.loaded=true;
    medical.urgent=true;
    medical.diagnoses={"pressure injury"};
    medical.bodyParts.push_back({"left arm",0.45f,false,{"deep cut"},{"reduced grasp"},""});
    medical.treatmentPlan={"stabilize","dress wound","reassess oxygenation"};
    std::cout<<DeveloperObservability::toText(DeveloperObservability::medical(medical))<<'\n';

    MilitaryReadinessSnapshot military{};
    military.squadStableId=430020;
    military.assignedMembers=6;
    military.presentMembers=5;
    military.equippedMembers=4;
    military.trainedMembers=6;
    military.requiredAmmo=180;
    military.availableAmmo=90;
    std::cout<<DeveloperObservability::toText(DeveloperObservability::military(military))<<'\n';

    const std::vector<ChronicleEventDiagnostic> events{
        {430100,100,"claim",430500,{430001},"founder registered frontier site",{}},
        {430101,110,"injury",430500,{430001},"founder injured during decompression",{430100}},
        {430102,120,"repair",430500,{430002},"atmosphere restored",{430101}}
    };
    std::cout<<DeveloperObservability::toText(DeveloperObservability::chronicle(430001,events,16))<<'\n';

    GraphicsTelemetrySnapshot gfx{};
    gfx.residentGpuBytes=32ULL*1024ULL*1024ULL;
    gfx.uploadedBytesThisFrame=256ULL*1024ULL;
    gfx.drawCalls=143;
    gfx.meshUploads=3;
    gfx.meshDestroys=1;
    gfx.triangles=182400;
    std::cout<<DeveloperObservability::toText(DeveloperObservability::graphics(gfx))<<'\n';

    FramePhaseTelemetrySnapshot frame{};
    frame.frameMs=16.67;
    frame.snapshotMs=0.35; frame.senseMs=1.1; frame.planMs=1.6; frame.resolveMs=2.4;
    frame.commitMs=0.75; frame.persistMs=0.2; frame.presentMs=4.1;
    frame.ecsEntities=1842; frame.persistentCommands=37;
    std::cout<<DeveloperObservability::toText(DeveloperObservability::framePhases(frame));
    return 0;
}

} // namespace

int main(int argc,char** argv) {
    try {
        if(argc<2) { usage(); return 2; }
        const std::string command=argv[1];
        if(command=="demo") return demo();
        if(command=="chunk-record") return chunkRecord(argc,argv);
        if(command=="worldgen-window") return worldgenWindow(argc,argv);
        if(command=="cross-section") return crossSection(argc,argv);
        if(command=="mesh") return mesh(argc,argv);
        if(command=="snapshot-demo") return snapshotDemo();
        usage();
        return 2;
    } catch(const std::exception& e) {
        std::cerr<<"elysium_inspect: "<<e.what()<<'\n';
        return 1;
    }
}
