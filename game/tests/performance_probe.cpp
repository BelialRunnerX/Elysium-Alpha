// Production performance/queue-boundary probe using the authoritative runtime APIs.
#include "core/JobSystem.hpp"
#include "render/GraphicsBackend.hpp"
#include "render/PlanetSurfaceRenderer.hpp"
#include "world/PlanetSurface.hpp"
#include "world/PlanetSurfaceMesher.hpp"
#include "world/SurfaceChunkCache.hpp"
#include "world/SurfaceChunkPersistence.hpp"
#include "world/SurfaceInfrastructure.hpp"
#include "world/SurfaceNavigation.hpp"
#include "world/SurfaceWorldRead.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace elysium;

namespace {

class ProbeGraphicsBackend final : public IGraphicsBackend {
public:
    GraphicsMeshHandle uploadMesh(const CpuMeshData& data) override {
        const auto id=next_++;
        live_[id]=data.vertexCount();
        ++uploads;
        uploadedVertices+=static_cast<std::uint64_t>(std::max(0,data.vertexCount()));
        return {id};
    }
    void destroyMesh(GraphicsMeshHandle handle) override {
        if(!handle) return;
        live_.erase(handle.value);
        ++destroys;
    }
    void drawMesh(GraphicsMeshHandle handle) const override {
        if(handle && live_.contains(handle.value)) ++draws;
    }

    mutable std::uint64_t draws{};
    std::uint64_t uploads{};
    std::uint64_t destroys{};
    std::uint64_t uploadedVertices{};
    std::size_t liveCount() const { return live_.size(); }
private:
    std::uint32_t next_{1};
    std::unordered_map<std::uint32_t,int> live_;
};

std::vector<PlanetChunkAddress> allSurfaceChunks() {
    std::vector<PlanetChunkAddress> out;
    out.reserve(PlanetSurface::ChunkCount);
    for(int face=0;face<PlanetSurface::FaceCount;++face)
        for(int cv=0;cv<PlanetSurface::ChunksPerFaceAxis;++cv)
            for(int cu=0;cu<PlanetSurface::ChunksPerFaceAxis;++cu)
                out.push_back({static_cast<CubeFace>(face),cu,cv,0});
    return out;
}

bool settleCache(SurfaceChunkCache& cache,const PlanetSurface& planet,int maxIterations=5000) {
    for(int i=0;i<maxIterations;++i) {
        cache.sync(planet);
        if(cache.stats().pending==0) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    cache.sync(planet);
    return cache.stats().pending==0;
}

bool settleRenderer(PlanetSurfaceRenderer& renderer,const PlanetSurface& planet,int maxIterations=5000) {
    for(int i=0;i<maxIterations;++i) {
        renderer.sync(planet);
        if(renderer.pendingJobs()==0 && renderer.dirtyChunks()==0) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    renderer.sync(planet);
    return renderer.pendingJobs()==0 && renderer.dirtyChunks()==0;
}

double elapsedMs(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
}

struct ProbeResults {
    SurfaceChunkCacheStats cache{};
    JobSystemStats rendererJobs{};
    SurfaceNavigationStats navigation{};
    double cacheBuildMs{};
    double directMeshAvgMs{};
    double directMeshMaxMs{};
    std::uint64_t directMeshSamples{};
    std::uint64_t directMeshQuads{};
    double rendererEditAvgMs{};
    double rendererEditMaxMs{};
    std::uint64_t rendererEditSamples{};
    double navigationAvgMs{};
    double navigationMaxMs{};
    int navigationMaxExpanded{};
    int rendererQuads{};
    int rendererTriangles{};
    std::uint64_t rendererUploads{};
    std::size_t dirtyChunks{};
    std::uintmax_t saveBytes{};
    double saveWriteMs{};
    bool queueBounded{};
    std::size_t hardQueueObserved{};
    std::uint64_t deferredVisible{};
    bool rendererSettled{};
    bool cacheSettled{};
};

ProbeResults runProbe() {
    ProbeResults out{};
    constexpr std::uint64_t seed=0x44E1A51ULL;
    PlanetSurface planet(seed,PlanetClass::Temperate);
    const auto addresses=allSurfaceChunks();

    // Serial generation produces a deterministic queue/frontier exercise while
    // wall-clock timing remains diagnostic only.
    JobSystem serial(SerialJobs);
    SurfaceChunkCache cache(serial,64,128U*1024U*1024U,8);
    const auto cacheStarted=std::chrono::steady_clock::now();
    std::size_t published=0;
    for(int round=0;round<64 && published<addresses.size();++round) {
        cache.beginFrame();
        for(const auto& a:addresses) (void)cache.request(planet,a,SurfaceChunkPriority::Visible);
        cache.sync(planet);
        published=static_cast<std::size_t>(cache.stats().published);
    }
    out.cacheSettled=settleCache(cache,planet) && cache.stats().published==addresses.size();
    out.cacheBuildMs=elapsedMs(cacheStarted);
    out.cache=cache.stats();

    double meshTotal=0.0;
    for(const auto& a:addresses) {
        const auto data=cache.find(a);
        if(!data) continue;
        const auto started=std::chrono::steady_clock::now();
        const auto mesh=buildPlanetSurfaceChunkMesh(*data);
        const auto ms=elapsedMs(started);
        meshTotal+=ms;
        out.directMeshMaxMs=std::max(out.directMeshMaxMs,ms);
        ++out.directMeshSamples;
        out.directMeshQuads+=static_cast<std::uint64_t>(std::max(0,mesh.quads));
    }
    if(out.directMeshSamples) out.directMeshAvgMs=meshTotal/static_cast<double>(out.directMeshSamples);

    // maxPending is a hard scheduling frontier for all request priorities.
    JobSystem oneWorker(1);
    SurfaceChunkCache bounded(oneWorker,64,128U*1024U*1024U,4);
    bounded.beginFrame();
    for(const auto& a:addresses) {
        if(!bounded.request(planet,a,SurfaceChunkPriority::Visible)) ++out.deferredVisible;
        out.hardQueueObserved=std::max<std::size_t>(out.hardQueueObserved,static_cast<std::size_t>(bounded.stats().pending));
    }
    out.queueBounded=bounded.stats().pending<=4 && out.hardQueueObserved<=4 && out.deferredVisible>0;
    (void)settleCache(bounded,planet);

    // Renderer edit-to-visible timing is measured around the public sync/settle
    // contract rather than depending on a retired internal instrumentation ABI.
    JobSystem renderJobs(2);
    ProbeGraphicsBackend graphics;
    PlanetSurfaceRenderer renderer(renderJobs,graphics);
    const Vec3 focus=planet.cellCenterPosition({CubeFace::PositiveZ,32,32,
                                                planet.surfaceRadial(CubeFace::PositiveZ,32,32)});
    renderer.setStreamingFocus(focus,6,9);
    out.rendererSettled=settleRenderer(renderer,planet);
    double editTotal=0.0;
    for(int i=0;i<12;++i) {
        const int u=24+(i%6);
        const int v=26+(i/6);
        const int top=planet.surfaceRadial(CubeFace::PositiveZ,u,v);
        const SurfaceCellAddress cell{CubeFace::PositiveZ,u,v,std::min(top+1,PlanetSurface::RadialLayers-1)};
        planet.set(cell,BlockType::SteelPlate,true);
        const auto started=std::chrono::steady_clock::now();
        if(!settleRenderer(renderer,planet)) { out.rendererSettled=false; break; }
        const auto ms=elapsedMs(started);
        editTotal+=ms;
        out.rendererEditMaxMs=std::max(out.rendererEditMaxMs,ms);
        ++out.rendererEditSamples;
    }
    if(out.rendererEditSamples) out.rendererEditAvgMs=editTotal/static_cast<double>(out.rendererEditSamples);
    out.rendererJobs=renderJobs.stats();
    out.rendererQuads=renderer.quads();
    out.rendererTriangles=renderer.triangles();
    out.rendererUploads=graphics.uploads;

    // Navigation timing includes a real miss followed by repeated cache hits.
    SurfaceChunkCache navCache(serial,4,16U*1024U*1024U,4);
    SurfaceWorldReadService read(planet,navCache);
    SurfaceNavigationService nav(planet,64,1024);
    const Vec3 startDir=faceGridCellDirection(CubeFace::PositiveZ,58,30,PlanetSurface::FaceResolution);
    const Vec3 goalDir=faceGridCellDirection(CubeFace::PositiveX,4,34,PlanetSurface::FaceResolution);
    const Vec3 start=startDir*(read.surfaceBoundaryRadius(startDir)+1.8f);
    const Vec3 goal=goalDir*(read.surfaceBoundaryRadius(goalDir)+1.8f);
    double navTotal=0.0;
    for(int i=0;i<128;++i) {
        const auto started=std::chrono::steady_clock::now();
        const auto route=nav.findPath(read,start,goal);
        const auto ms=elapsedMs(started);
        navTotal+=ms;
        out.navigationMaxMs=std::max(out.navigationMaxMs,ms);
        out.navigationMaxExpanded=std::max(out.navigationMaxExpanded,route.expanded);
    }
    out.navigationAvgMs=navTotal/128.0;
    out.navigation=nav.stats();

    // Sparse persistence remains per-touched-chunk only.
    SurfaceInfrastructure infrastructure(seed);
    const auto temp=std::filesystem::temp_directory_path()/"elysium_performance_probe";
    std::error_code ec;
    std::filesystem::remove_all(temp,ec);
    SurfaceChunkStore store(temp,0,seed,kSurfaceGeneratorVersion,kSurfaceGeneratorFingerprint);
    for(int face=0;face<2;++face) {
        for(int u=4;u<60;u+=7) {
            for(int v=5;v<60;v+=11) {
                const auto f=static_cast<CubeFace>(face);
                const int r=planet.surfaceRadial(f,u,v);
                planet.set({f,u,v,r},BlockType::Air);
            }
        }
    }
    out.dirtyChunks=planet.journalCount();
    const auto saveStarted=std::chrono::steady_clock::now();
    for(const auto& [_,journal]:planet.journals()) {
        const auto record=makeSurfaceChunkRecord(0,planet,infrastructure,journal.address,1);
        std::string error;
        if(store.save(record,&error)) {
            const auto path=store.recordPath(journal.address);
            if(std::filesystem::exists(path)) out.saveBytes+=std::filesystem::file_size(path);
        }
    }
    out.saveWriteMs=elapsedMs(saveStarted);
    std::filesystem::remove_all(temp,ec);
    return out;
}

std::string markdown(const ProbeResults& r) {
    std::ostringstream out;
    out<<std::fixed<<std::setprecision(3);
    out<<"# Elysium Production Performance Probe\n\n";
    out<<"Wall-clock values are environment-specific diagnostics; deterministic acceptance is covered separately.\n\n";
    out<<"| Metric | Measured | Direction |\n";
    out<<"|---|---:|---:|\n";
    out<<"| Full bounded chunk-cache populate | "<<r.cacheBuildMs<<" ms | diagnostic |\n";
    out<<"| Direct LOD0 chunk mesh average | "<<r.directMeshAvgMs<<" ms | ~<3 ms avg off-thread |\n";
    out<<"| Direct LOD0 chunk mesh max | "<<r.directMeshMaxMs<<" ms | diagnose spikes |\n";
    out<<"| Renderer edit->visible average | "<<r.rendererEditAvgMs<<" ms | <50 ms desirable |\n";
    out<<"| Renderer edit->visible max | "<<r.rendererEditMaxMs<<" ms | <50 ms desirable |\n";
    out<<"| Chunk-cache hard frontier | "<<r.hardQueueObserved<<" pending | <=4 in stress fixture |\n";
    out<<"| Deferred visible requests | "<<r.deferredVisible<<" | expected under burst |\n";
    out<<"| Navigation average request | "<<r.navigationAvgMs<<" ms | hierarchical/bounded |\n";
    out<<"| Navigation max request | "<<r.navigationMaxMs<<" ms | bounded |\n";
    out<<"| Navigation max expanded | "<<r.navigationMaxExpanded<<" nodes | <= configured cap |\n";
    out<<"| Dirty save chunks | "<<r.dirtyChunks<<" | incremental only |\n";
    out<<"| Dirty chunk sidecar bytes | "<<r.saveBytes<<" bytes | incremental only |\n";
    out<<"| Dirty chunk save write time | "<<r.saveWriteMs<<" ms | incremental atomic IO |\n\n";
    out<<"## Counters\n\n";
    out<<"- Chunk generation requests/published: "<<r.cache.generationRequests<<" / "<<r.cache.published<<".\n";
    out<<"- Worker jobs submitted/completed: "<<r.rendererJobs.submitted<<" / "<<r.rendererJobs.completed<<".\n";
    out<<"- Renderer geometry: "<<r.rendererQuads<<" quads / "<<r.rendererTriangles<<" triangles; "<<r.rendererUploads<<" uploads.\n";
    out<<"- Navigation requests/hits/misses: "<<r.navigation.requests<<" / "<<r.navigation.cacheHits<<" / "<<r.navigation.cacheMisses<<".\n";
    out<<"- Queue frontier bounded: "<<(r.queueBounded?"yes":"NO")<<". Cache settled: "<<(r.cacheSettled?"yes":"NO")
       <<". Renderer settled: "<<(r.rendererSettled?"yes":"NO")<<".\n";
    return out.str();
}

} // namespace

int main(int argc,char** argv) {
    try {
        const auto result=runProbe();
        const auto report=markdown(result);
        std::cout<<report;
        if(argc>=3 && std::string(argv[1])=="--output") {
            std::ofstream file(argv[2],std::ios::binary|std::ios::trunc);
            if(!file) {
                std::cerr<<"Could not open performance report output: "<<argv[2]<<'\n';
                return 2;
            }
            file<<report;
        }
        if(!result.queueBounded || !result.cacheSettled || !result.rendererSettled) return 3;
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"Elysium performance probe failed: "<<e.what()<<'\n';
        return 1;
    }
}
