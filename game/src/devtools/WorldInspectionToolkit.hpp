#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
namespace elysium::devtools {
struct ScalarMapSample { std::uint64_t stableKey{}; double a{},b{},c{},d{}; };
struct MeshInspection { std::uint64_t chunkKey{}; std::uint64_t sourceRevision{}; std::uint64_t faces{}; std::uint64_t sealedRejected{}; std::uint64_t greedyQuads{}; std::uint64_t microQuads{}; std::uint64_t aoDarkenedCorners{}; };
struct PersistenceDiff { std::uint64_t spatialKey{}; std::uint64_t baselineHash{}; std::uint64_t resolvedHash{}; std::uint64_t deltaCount{}; std::uint64_t tombstones{}; bool unexpectedFullWrite{}; };
class WorldInspectionToolkit { public: using MapProvider=std::function<std::vector<ScalarMapSample>()>; void addMapProvider(std::string name,MapProvider); void addMesh(MeshInspection); void addDiff(PersistenceDiff); std::string deterministicReport() const; private: std::vector<std::pair<std::string,MapProvider>> maps_; std::vector<MeshInspection> meshes_; std::vector<PersistenceDiff> diffs_; };
}
