// Save tests.
//
// A save system's failure mode is losing somebody's work, and it usually does
// it quietly and much later than the mistake. So these tests are less about the
// happy path than about the ways a journal goes wrong: a torn write from a
// crash, a file from a different planet, a generator that has moved under the
// edits, and the ordering question — a voxel edited twice must end up with the
// second value, whatever order the records happen to be walked in.
#include "harness.h"

#include "save/editstore.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace ely;
using elytest::section;

namespace {

constexpr uint64_t kSeed = 424242;
constexpr uint64_t kFingerprint = 0x9ff58a3b49db9919ull;

std::string tempPath(const char* name) {
    return std::string("/tmp/elysium_test_") + name + ".edits";
}

void removeFile(const std::string& path) { std::remove(path.c_str()); }

size_t fileSize(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return 0;
    std::fseek(f, 0, SEEK_END);
    const long n = std::ftell(f);
    std::fclose(f);
    return n < 0 ? 0 : size_t(n);
}

// Chop bytes off the end, which is what a crash mid-write leaves behind.
void truncateFile(const std::string& path, size_t bytesToDrop) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return;
    std::fseek(f, 0, SEEK_END);
    const size_t size = size_t(std::ftell(f));
    std::fseek(f, 0, SEEK_SET);
    std::vector<uint8_t> data(size);
    if (std::fread(data.data(), 1, size, f) != size) data.clear();
    std::fclose(f);
    if (data.size() <= bytesToDrop) return;
    data.resize(data.size() - bytesToDrop);
    f = std::fopen(path.c_str(), "wb");
    std::fwrite(data.data(), 1, data.size(), f);
    std::fclose(f);
}

// Flip a byte in the middle, which is what bit rot or a bad cable leaves.
void corruptByte(const std::string& path, size_t offset) {
    std::FILE* f = std::fopen(path.c_str(), "r+b");
    if (!f) return;
    std::fseek(f, long(offset), SEEK_SET);
    int c = std::fgetc(f);
    std::fseek(f, long(offset), SEEK_SET);
    std::fputc(c ^ 0x5A, f);
    std::fclose(f);
}

}  // namespace

static void testAddressing() {
    section("addressing");

    // A chunk address must be a property of the planet, not of wherever a
    // player happened to start. Metres map to chunks by flooring, so negative
    // coordinates must round the same way positive ones do — otherwise the
    // chunk at -1 m and the chunk at +1 m are the same chunk.
    const ChunkAddress a = ChunkAddress::fromMetres(2, 0.0, 0.0, 0.0);
    const ChunkAddress b = ChunkAddress::fromMetres(2, 31.9, 31.9, 31.9);
    const ChunkAddress c = ChunkAddress::fromMetres(2, 32.1, 0.0, 0.0);
    const ChunkAddress d = ChunkAddress::fromMetres(2, -0.1, 0.0, 0.0);
    CHECK(a == b, "two points in the same chunk gave different addresses");
    CHECK(!(a == c), "a point in the next chunk gave the same address");
    CHECK(d.x == -1, "a point just below zero landed in chunk %d, expected -1",
          d.x);

    // Voxel addresses must round-trip, and a block address must never collide
    // with a micro address in the same block.
    bool blocksOk = true, microOk = true, collision = false;
    for (int y = 0; y < kChunkSize; y += 7)
        for (int z = 0; z < kChunkSize; z += 5)
            for (int x = 0; x < kChunkSize; x += 3) {
                const VoxelAddress block = VoxelAddress::wholeBlock(x, y, z);
                if (block.isMicro()) blocksOk = false;
                if (block.blockIndex() != VoxelAddress::blockIndexOf(x, y, z))
                    blocksOk = false;
                for (int m = 0; m < kMicro; m += 5) {
                    const VoxelAddress micro =
                        VoxelAddress::microVoxel(x, y, z, m, m, m);
                    if (!micro.isMicro()) microOk = false;
                    if (micro.blockIndex() != block.blockIndex()) microOk = false;
                    if (micro.bits == block.bits) collision = true;
                }
            }
    CHECK(blocksOk, "block addresses do not round-trip");
    CHECK(microOk, "micro addresses do not round-trip or lose their block");
    CHECK(!collision, "a micro address collided with a whole-block address");
}

static void testInMemory() {
    section("edits in memory");

    EditStore store;
    const ChunkAddress chunk{1, 4, -2, 7};
    Material got = MAT_AIR;

    CHECK(store.empty(), "a new store is not empty");
    CHECK(!store.lookupBlock(chunk, 1, 2, 3, got),
          "an untouched voxel reported an edit");

    store.setBlock(chunk, 1, 2, 3, MAT_STONE);
    CHECK(store.lookupBlock(chunk, 1, 2, 3, got) && got == MAT_STONE,
          "an edit did not read back");
    CHECK(!store.lookupBlock(chunk, 1, 2, 4, got),
          "an edit leaked into the neighbouring voxel");
    CHECK(store.editedChunkCount() == 1 && store.editCount() == 1,
          "one edit produced %zu chunks and %zu edits",
          store.editedChunkCount(), store.editCount());

    // Editing the same voxel again replaces it rather than accumulating.
    store.setBlock(chunk, 1, 2, 3, MAT_AIR);
    CHECK(store.lookupBlock(chunk, 1, 2, 3, got) && got == MAT_AIR,
          "the second edit to a voxel did not win");
    CHECK(store.editCount() == 1, "re-editing a voxel produced %zu entries",
          store.editCount());

    // A micro edit must be finer than, and win over, a block edit on the same
    // block: it is the later and more specific statement about that space.
    store.setBlock(chunk, 5, 5, 5, MAT_STONE);
    store.setMicro(chunk, 5, 5, 5, 2, 2, 2, MAT_AIR);
    CHECK(store.lookupMicro(chunk, 5, 5, 5, 2, 2, 2, got) && got == MAT_AIR,
          "a micro edit did not win over the block edit under it");
    CHECK(store.lookupMicro(chunk, 5, 5, 5, 3, 3, 3, got) && got == MAT_STONE,
          "an unedited micro-voxel did not fall through to the block edit");
}

static void testRoundTrip() {
    section("journal round trip");

    const std::string path = tempPath("roundtrip");
    removeFile(path);

    // Write a spread of edits across several chunks and both resolutions.
    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        for (int c = 0; c < 6; ++c) {
            const ChunkAddress chunk{uint8_t(c % 6), c, -c, c * 3};
            for (int i = 0; i < 40; ++i)
                store.setBlock(chunk, i % 32, (i * 3) % 32, (i * 7) % 32,
                               Material(MAT_STONE + (i % 5)));
            store.setMicro(chunk, 1, 1, 1, c, c, c, MAT_AIR);
        }
        std::string error;
        CHECK(store.flush(&error), "flush failed: %s", error.c_str());
        CHECK(store.pendingCount() == 0, "%zu edits were still pending after a"
              " flush", store.pendingCount());
    }

    // Read them back into a fresh store.
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(report.fileExisted && report.headerValid,
              "the journal did not load: %s", report.message.c_str());
        CHECK(report.fingerprintMatches, "the fingerprint did not match");
        CHECK(!report.truncatedTail, "a clean file reported a torn tail");
        CHECK(store.editedChunkCount() == 6,
              "%zu chunks came back, expected 6", store.editedChunkCount());

        Material got = MAT_AIR;
        bool allBack = true;
        for (int c = 0; c < 6; ++c) {
            const ChunkAddress chunk{uint8_t(c % 6), c, -c, c * 3};
            for (int i = 0; i < 40; ++i) {
                if (!store.lookupBlock(chunk, i % 32, (i * 3) % 32, (i * 7) % 32,
                                       got))
                    allBack = false;
            }
            if (!store.lookupMicro(chunk, 1, 1, 1, c, c, c, got) || got != MAT_AIR)
                allBack = false;
        }
        CHECK(allBack, "some edits did not survive the round trip");
    }
    removeFile(path);
}

static void testAppendAndOrdering() {
    section("appending and ordering");

    const std::string path = tempPath("append");
    removeFile(path);
    const ChunkAddress chunk{0, 1, 1, 1};

    // Three sessions, each editing the same voxel. The last one must win, which
    // is the entire semantics of a journal.
    for (int session = 0; session < 3; ++session) {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        store.setBlock(chunk, 9, 9, 9, Material(MAT_STONE + session));
        std::string error;
        CHECK_QUIET(store.flush(&error));
    }

    EditStore store;
    store.load(path, kSeed, kFingerprint);
    Material got = MAT_AIR;
    CHECK(store.lookupBlock(chunk, 9, 9, 9, got),
          "the voxel had no edit after three sessions");
    CHECK(got == Material(MAT_STONE + 2),
          "the journal replayed to material %d; the last write must win", int(got));
    CHECK(store.editCount() == 1, "three writes to one voxel left %zu live edits",
          store.editCount());
    removeFile(path);
}

static void testTornTail() {
    section("a crash mid-write");

    const std::string path = tempPath("torn");
    removeFile(path);

    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        for (int i = 0; i < 5; ++i) {
            store.setBlock(ChunkAddress{0, i, 0, 0}, 1, 1, 1, MAT_STONE);
            std::string error;
            CHECK_QUIET(store.flush(&error));   // one record per flush
        }
    }
    const size_t whole = fileSize(path);
    CHECK(whole > 0, "the journal is empty");

    // Chop the last record in half, which is what a process killed mid-write
    // leaves behind.
    truncateFile(path, 9);
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(report.truncatedTail, "a torn tail was not detected");
        CHECK(report.recordsRead == 4,
              "%llu records survived the tear, expected 4",
              (unsigned long long)report.recordsRead);
        CHECK(store.editedChunkCount() == 4,
              "%zu chunks survived; the player must lose one edit, not the"
              " planet", store.editedChunkCount());
    }

    // And a plausible-looking record whose contents are wrong. This is the one
    // that a length check alone would miss and that would corrupt a world
    // silently, which is why every record carries a checksum.
    {
        EditStore fresh;
        removeFile(path);
        fresh.load(path, kSeed, kFingerprint);
        for (int i = 0; i < 5; ++i) {
            fresh.setBlock(ChunkAddress{0, i, 0, 0}, 1, 1, 1, MAT_STONE);
            std::string error;
            CHECK_QUIET(fresh.flush(&error));
        }
    }
    const size_t headerBytes = 28;
    corruptByte(path, headerBytes + 2);   // inside the first record
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(report.truncatedTail,
              "a record with a bad checksum was accepted");
        CHECK(report.recordsRead == 0,
              "%llu records were read past a corrupt one; replay must stop at"
              " the first failure", (unsigned long long)report.recordsRead);
    }
    removeFile(path);
}

static void testWrongWorld() {
    section("journals from another world");

    const std::string path = tempPath("wrongworld");
    removeFile(path);
    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        store.setBlock(ChunkAddress{0, 0, 0, 0}, 1, 1, 1, MAT_STONE);
        std::string error;
        CHECK_QUIET(store.flush(&error));
    }

    // A different planet's journal must be refused outright: applying it would
    // scatter one world's edits across another.
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed + 1, kFingerprint);
        CHECK(store.empty(), "a journal from another planet was applied");
        CHECK(!report.message.empty(), "the refusal was not explained");
    }

    // A changed generator must be flagged but still loaded. The edits are good
    // data about a world that no longer exists; silently applying them would
    // leave a player's doorway floating in mid-air with no explanation, and
    // silently discarding them would throw their work away.
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint ^ 1);
        CHECK(!report.fingerprintMatches,
              "a changed generator was not detected");
        CHECK(!store.empty(),
              "a changed generator threw the player's edits away");
    }

    // Garbage is refused rather than guessed at.
    {
        std::FILE* f = std::fopen(path.c_str(), "wb");
        const char junk[] = "this is not an edit journal at all, not even close";
        std::fwrite(junk, 1, sizeof(junk), f);
        std::fclose(f);
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(!report.headerValid, "garbage was accepted as a header");
        CHECK(store.empty(), "garbage produced edits");
    }
    removeFile(path);
}

static void testCompaction() {
    section("compaction");

    const std::string path = tempPath("compact");
    removeFile(path);

    // Edit the same small set of voxels many times over. On disk that is a long
    // journal; live, it is a handful of edits.
    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        for (int pass = 0; pass < 80; ++pass) {
            for (int i = 0; i < 4; ++i)
                store.setBlock(ChunkAddress{0, 0, 0, 0}, i, 0, 0,
                               Material(MAT_STONE + (pass % 4)));
            std::string error;
            CHECK_QUIET(store.flush(&error));
        }
        CHECK(store.shouldCompact(),
              "80 records for 4 live edits did not ask for compaction");

        const size_t before = fileSize(path);
        std::string error;
        CHECK(store.compact(&error), "compaction failed: %s", error.c_str());
        const size_t after = fileSize(path);
        CHECK(after < before, "compaction grew the file: %zu to %zu bytes",
              before, after);
        CHECK(!store.shouldCompact(),
              "the file still wants compacting immediately afterwards");
    }

    // And it must not have changed what the world looks like.
    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(!report.truncatedTail, "the compacted file has a torn tail");
        CHECK(store.editCount() == 4,
              "%zu edits survived compaction, expected 4", store.editCount());
        Material got = MAT_AIR;
        bool ok = true;
        for (int i = 0; i < 4; ++i) {
            if (!store.lookupBlock(ChunkAddress{0, 0, 0, 0}, i, 0, 0, got) ||
                got != Material(MAT_STONE + (79 % 4)))
                ok = false;
        }
        CHECK(ok, "compaction changed the surviving values");
    }
    removeFile(path);
}

static void testCostIsProportionalToChanges() {
    section("cost tracks what was changed, not where you went");

    // The whole point of the paradigm. A planet is billions of voxels; visiting
    // it must cost nothing, and cutting one doorway must cost one doorway.
    const std::string path = tempPath("cost");
    removeFile(path);

    {
        EditStore store;
        const LoadReport report = store.load(path, kSeed, kFingerprint);
        CHECK(!report.fileExisted,
              "an unvisited planet already had a save file");
        CHECK(store.empty(), "an unvisited planet had edits");
        std::string error;
        CHECK(store.flush(&error), "flushing nothing failed");
        CHECK(fileSize(path) == 0,
              "visiting a planet and changing nothing wrote %zu bytes",
              fileSize(path));
    }

    // Now dig a doorway: a 2 x 3 x 1 hole.
    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        const ChunkAddress chunk{3, 12, 0, -5};
        for (int y = 0; y < 3; ++y)
            for (int x = 0; x < 2; ++x)
                store.setBlock(chunk, x, y, 0, MAT_AIR);
        std::string error;
        CHECK(store.flush(&error), "flush failed: %s", error.c_str());
    }

    const size_t size = fileSize(path);
    CHECK(size > 0 && size < 128,
          "a six-voxel doorway cost %zu bytes; it should be tens", size);

    {
        EditStore store;
        store.load(path, kSeed, kFingerprint);
        Material got = MAT_STONE;
        const ChunkAddress chunk{3, 12, 0, -5};
        CHECK(store.lookupBlock(chunk, 0, 0, 0, got) && got == MAT_AIR,
              "the doorway was not there when we came back");
        CHECK(store.editCount() == 6, "%zu voxels came back, expected 6",
              store.editCount());
    }
    std::printf("    an unvisited planet: 0 bytes. A six-voxel doorway: %zu"
                " bytes.\n", size);
    removeFile(path);
}

int main() {
    std::printf("Elysium — save tests\n");
    testAddressing();
    testInMemory();
    testRoundTrip();
    testAppendAndOrdering();
    testTornTail();
    testWrongWorld();
    testCompaction();
    testCostIsProportionalToChanges();
    return elytest::report("save tests");
}
