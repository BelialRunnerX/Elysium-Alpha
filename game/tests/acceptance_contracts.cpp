// Intended function: imported tests implementation for acceptance_contracts; preserves the agent-authored subsystem contract for later integration/debugging.
#include "core/Determinism.hpp"
#include "core/JobSystem.hpp"
#include "render/GraphicsBackend.hpp"
#include "world/InfrastructureJournal.hpp"
#include "world/PlanetSurface.hpp"
#include "world/SurfaceChunkPersistence.hpp"
#include "world/SurfaceInfrastructure.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using namespace elysium;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

std::uint64_t fnvAppendU64(std::uint64_t h, std::uint64_t value) {
    for (int byte = 0; byte < 8; ++byte) {
        h ^= (value >> (byte * 8)) & 0xffULL;
        h *= 1099511628211ULL;
    }
    return h;
}

void testGeneratorFixedCorpus() {
    // This is deliberately a fixed corpus, not merely "two objects created in
    // the same process agree". If deterministic baseline behavior changes,
    // the generator version/fingerprint and this corpus must be updated in the
    // same intentional compatibility change.
    struct Case { std::uint64_t seed; PlanetClass planetClass; };
    constexpr Case cases[] {
        {0x51A7E5EEDULL, PlanetClass::Temperate},
        {0x0BADC0FFEEULL, PlanetClass::Barren},
        {0x00C0FFEE42ULL, PlanetClass::Scorched}
    };
    constexpr int samples[][2] {{0,0},{7,13},{31,31},{32,17},{63,63},{62,1}};

    std::uint64_t digest = 1469598103934665603ULL;
    for (const auto& c : cases) {
        PlanetSurface planet(c.seed, c.planetClass);
        digest = fnvAppendU64(digest, c.seed);
        digest = fnvAppendU64(digest, static_cast<std::uint64_t>(c.planetClass));
        digest = fnvAppendU64(digest, static_cast<std::uint64_t>(PlanetSurface::GeneratorVersion));
        digest = fnvAppendU64(digest, PlanetSurface::GeneratorFingerprint);
        for (int faceIndex = 0; faceIndex < PlanetSurface::FaceCount; ++faceIndex) {
            const auto face = static_cast<CubeFace>(faceIndex);
            for (const auto& uv : samples) {
                const int surface = planet.surfaceRadial(face, uv[0], uv[1]);
                digest = fnvAppendU64(digest, static_cast<std::uint64_t>(faceIndex));
                digest = fnvAppendU64(digest, static_cast<std::uint64_t>(uv[0]));
                digest = fnvAppendU64(digest, static_cast<std::uint64_t>(uv[1]));
                digest = fnvAppendU64(digest, static_cast<std::uint64_t>(surface));
                for (int delta = -3; delta <= 1; ++delta) {
                    const int radial = surface + delta;
                    if (!planet.radialInBounds(radial)) continue;
                    digest = fnvAppendU64(digest, static_cast<std::uint64_t>(planet.get(face, uv[0], uv[1], radial)));
                }
            }
        }
    }

    constexpr std::uint64_t expectedV1ElySph01 = 0xfe373cd5e68ffce8ULL;
    require(PlanetSurface::GeneratorVersion == 1,
            "generator version changed without updating the fixed acceptance corpus");
    require(PlanetSurface::GeneratorFingerprint == 0x454C595350483031ULL,
            "generator fingerprint changed without updating the fixed acceptance corpus");
    require(digest == expectedV1ElySph01,
            "fixed seed/version generator corpus drifted; bump version/fingerprint intentionally or restore baseline behavior");
}

void testSealedVolumeBudgetContract() {
    PlanetSurface planet(0x5EA1B0D6E7ULL, PlanetClass::Barren);
    const CubeFace face = CubeFace::PositiveZ;
    constexpr int u0 = 18;
    constexpr int v0 = 18;
    constexpr int r0 = 24;

    // 5^3 shell -> 3^3 = 27 reachable interior gas cells.
    for (int r = r0; r < r0 + 5; ++r) {
        for (int v = v0; v < v0 + 5; ++v) {
            for (int u = u0; u < u0 + 5; ++u) {
                const bool shell = u == u0 || u == u0 + 4 || v == v0 || v == v0 + 4 || r == r0 || r == r0 + 4;
                planet.set({face,u,v,r}, shell ? BlockType::SteelPlate : BlockType::Air, true);
            }
        }
    }

    const SurfaceCellAddress inside{face,u0+2,v0+2,r0+2};

    // Authoritative integration rule: termination *before* the configured
    // budget is sealed; reaching/exceeding the budget is open/unbounded.
    const auto exactBudget = planet.sealedVolume(inside, 27);
    require(!exactBudget.sealed && exactBudget.truncated && exactBudget.cells.size() == 27,
            "sealed-volume query treated budget exhaustion as a sealed room");

    const auto oneSpareCell = planet.sealedVolume(inside, 28);
    require(oneSpareCell.sealed && !oneSpareCell.truncated && oneSpareCell.cells.size() == 27,
            "sealed-volume query failed to recognize a naturally terminating bounded room");

    planet.set({face,u0+2,v0+2,r0+4}, BlockType::Air, true);
    const auto breached = planet.sealedVolume(inside, 512);
    require(!breached.sealed,
            "breached room was reported sealed after a path to the unbounded exterior was opened");
}

void testPersistenceNegativeRejection() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "elysium_agent42_acceptance_persistence";
    std::error_code ec;
    fs::remove_all(root, ec);

    PlanetSurface planet(0xA42A42A42ULL, PlanetClass::Temperate);
    SurfaceInfrastructure infrastructure(planet.seed());
    const PlanetChunkAddress owner{CubeFace::PositiveZ,0,0,0};
    const int radial = planet.surfaceRadial(owner.face, 5, 6);
    planet.set({owner.face,5,6,radial}, BlockType::SteelPlate, true);

    SurfaceChunkStore store(root, 3, planet.seed(), kSurfaceGeneratorVersion, kSurfaceGeneratorFingerprint);
    auto record = makeSurfaceChunkRecord(3, planet, infrastructure, owner, 77);
    std::string error;
    require(store.save(record, &error), "valid acceptance chunk could not be saved: " + error);

    // Compatibility mismatch must be rejected rather than silently applying
    // deltas to another baseline/world identity.
    SurfaceChunkStore wrongFingerprint(root, 3, planet.seed(), kSurfaceGeneratorVersion,
                                       kSurfaceGeneratorFingerprint ^ 0x55ULL);
    const auto incompatible = wrongFingerprint.load(owner, 77);
    require(!incompatible.loaded && incompatible.error.find("compatibility/identity mismatch") != std::string::npos,
            "chunk with wrong generator fingerprint was accepted");

    // Expected whole-save generation is a transaction boundary, not advisory.
    const auto wrongGeneration = store.load(owner, 78);
    require(!wrongGeneration.loaded && wrongGeneration.error.find("save-generation mismatch") != std::string::npos,
            "chunk from a different whole-save generation was accepted");

    // A record that claims another store identity must fail before touching disk.
    auto wrongStoreRecord = record;
    wrongStoreRecord.planetSlot = 4;
    error.clear();
    require(!store.save(wrongStoreRecord, &error) && error.find("identity") != std::string::npos,
            "store accepted a record from the wrong planet slot");

    // With no previous generation available, a torn current transaction must
    // be loud and must not yield partially parsed state.
    fs::remove(store.previousPath(owner), ec);
    {
        std::ofstream torn(store.recordPath(owner), std::ios::binary | std::ios::trunc);
        torn << "ELYSIUM_CHUNK_TXN 8\npayload_bytes 100\nchecksum 1\n\nTORN";
    }
    const auto tornLoad = store.load(owner, 77);
    require(!tornLoad.loaded &&
            (tornLoad.error.find("truncated") != std::string::npos || tornLoad.error.find("checksum") != std::string::npos),
            "torn chunk transaction was not rejected when no known-good fallback existed");

    // Legacy plain manifests are readable only when the caller explicitly
    // allows that compatibility path.
    const fs::path legacy = root / "legacy_manifest.txt";
    {
        std::ofstream out(legacy, std::ios::binary | std::ios::trunc);
        out << "ELYSIUM_SAVE 6 1\nEND\n";
    }
    const auto legacyDisabled = readCheckedAtomicGeneration(legacy, false);
    require(!legacyDisabled.loaded,
            "plain legacy manifest was accepted when legacy compatibility was explicitly disabled");

    fs::remove_all(root, ec);
}

void testInfrastructureRecordNegativeValidation() {
    SurfaceMachineObject machine{};
    machine.stableId = 0x4201ULL;
    machine.type = MachineType::StorageCrate;
    machine.anchor = {CubeFace::PositiveX,4096,4173,73};
    auto record = makeInfrastructureUpsert(machine);
    std::string error;
    require(validateInfrastructureJournalRecord(record, &error),
            "valid large-address infrastructure record failed acceptance setup: " + error);

    auto zeroId = record;
    zeroId.stableId = 0;
    require(!validateInfrastructureJournalRecord(zeroId, &error),
            "infrastructure record accepted zero stable identity");

    auto kindMismatch = record;
    kindMismatch.kind = InfrastructureRecordKind::Portal;
    require(!validateInfrastructureJournalRecord(kindMismatch, &error),
            "infrastructure record accepted a payload whose variant does not match its declared kind");

    auto tombstoneWithPayload = makeInfrastructureTombstone(InfrastructureRecordKind::Machine,
                                                             machine.stableId, machine.anchor);
    tombstoneWithPayload.payload = machine;
    require(!validateInfrastructureJournalRecord(tombstoneWithPayload, &error),
            "infrastructure tombstone accepted a payload that could resurrect stale object state");
}

class LifetimeCheckingBackend final : public IGraphicsBackend {
public:
    GraphicsMeshHandle uploadMesh(const CpuMeshData&) override {
        const auto handle = GraphicsMeshHandle{next_++};
        live_.emplace(handle.value, true);
        return handle;
    }

    void destroyMesh(GraphicsMeshHandle handle) override {
        if (!handle) return;
        live_.erase(handle.value);
    }

    void drawMesh(GraphicsMeshHandle handle) const override {
        if (handle && live_.contains(handle.value)) ++drawCount_;
        else ++rejectedDrawCount_;
    }

    int drawCount() const { return drawCount_; }
    int rejectedDrawCount() const { return rejectedDrawCount_; }

private:
    std::uint32_t next_{1};
    std::unordered_map<std::uint32_t, bool> live_;
    mutable int drawCount_{};
    mutable int rejectedDrawCount_{};
};

void testRendererStubLifetimeContract() {
    LifetimeCheckingBackend backend;
    CpuMeshData packet{};
    const auto handle = backend.uploadMesh(packet);
    require(static_cast<bool>(handle), "fake graphics backend returned a null live handle");
    backend.drawMesh(handle);
    require(backend.drawCount() == 1 && backend.rejectedDrawCount() == 0,
            "live fake graphics handle was rejected");

    backend.destroyMesh(handle);
    backend.drawMesh(handle);
    require(backend.drawCount() == 1 && backend.rejectedDrawCount() == 1,
            "destroyed graphics handle was accepted by the renderer contract stub");

    // Opaque backend handles are process-local presentation state. The only
    // identity property the portable contract relies on is zero == invalid.
    require(!static_cast<bool>(GraphicsMeshHandle{}), "zero graphics handle lost invalid-sentinel semantics");
}

struct TestCase {
    std::string_view name;
    void (*run)();
};

constexpr TestCase kTests[] {
    {"generator-fixed-corpus", testGeneratorFixedCorpus},
    {"sealed-volume-budget", testSealedVolumeBudgetContract},
    {"persistence-negative-rejection", testPersistenceNegativeRejection},
    {"infrastructure-negative-validation", testInfrastructureRecordNegativeValidation},
    {"renderer-stub-lifetime", testRendererStubLifetimeContract},
};

int runOne(const TestCase& test) {
    try {
        test.run();
        std::cout << "PASS " << test.name << '\n';
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL " << test.name << ": " << e.what() << '\n';
        return 1;
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc == 3 && std::string_view(argv[1]) == "--case") {
        const std::string_view requested = argv[2];
        const auto it = std::find_if(std::begin(kTests), std::end(kTests),
                                     [&](const TestCase& t){ return t.name == requested; });
        if (it == std::end(kTests)) {
            std::cerr << "Unknown acceptance case: " << requested << '\n';
            return 2;
        }
        return runOne(*it);
    }

    if (argc == 2 && std::string_view(argv[1]) == "--list") {
        for (const auto& test : kTests) std::cout << test.name << '\n';
        return 0;
    }

    int failed = 0;
    for (const auto& test : kTests) failed += runOne(test);
    if (failed == 0) std::cout << "Elysium acceptance contracts: PASS\n";
    else std::cerr << "Elysium acceptance contracts: FAIL (" << failed << ")\n";
    return failed == 0 ? 0 : 1;
}
