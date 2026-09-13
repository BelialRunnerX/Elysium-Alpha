#include "devtools/WorldInspectionToolkit.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace elysium::devtools {

void WorldInspectionToolkit::addMapProvider(std::string name, MapProvider provider) {
    if (!name.empty() && provider) maps_.push_back({std::move(name), std::move(provider)});
}

void WorldInspectionToolkit::addMesh(MeshInspection value) { meshes_.push_back(value); }
void WorldInspectionToolkit::addDiff(PersistenceDiff value) { diffs_.push_back(value); }

std::string WorldInspectionToolkit::deterministicReport() const {
    std::ostringstream out;
    out << std::setprecision(9);

    auto maps = maps_;
    std::sort(maps.begin(), maps.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    for (auto& map : maps) {
        auto rows = map.second();
        std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) { return a.stableKey < b.stableKey; });
        for (const auto& row : rows) {
            out << "MAP," << map.first << ',' << row.stableKey << ',' << row.a << ',' << row.b << ',' << row.c << ',' << row.d << '\n';
        }
    }

    auto meshes = meshes_;
    std::sort(meshes.begin(), meshes.end(), [](const auto& a, const auto& b) { return a.chunkKey < b.chunkKey; });
    for (const auto& row : meshes) {
        out << "MESH," << row.chunkKey << ',' << row.sourceRevision << ',' << row.faces << ',' << row.sealedRejected << ','
            << row.greedyQuads << ',' << row.microQuads << ',' << row.aoDarkenedCorners << '\n';
    }

    auto diffs = diffs_;
    std::sort(diffs.begin(), diffs.end(), [](const auto& a, const auto& b) { return a.spatialKey < b.spatialKey; });
    for (const auto& row : diffs) {
        out << "DIFF," << row.spatialKey << ',' << row.baselineHash << ',' << row.resolvedHash << ',' << row.deltaCount << ','
            << row.tombstones << ',' << row.unexpectedFullWrite << '\n';
    }
    return out.str();
}

} // namespace elysium::devtools
