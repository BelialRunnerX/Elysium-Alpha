#include "editstore.h"

#include <zlib.h>

#include <cmath>
#include <cstdio>
#include <cstring>

namespace ely {

namespace {

// "ELYEDIT1" — magic and version in one word, so a file from a future format is
// rejected by the magic check rather than misparsed.
constexpr uint64_t kMagic = 0x3144494545594C45ull;
constexpr uint32_t kFormatVersion = 1;

// Header: magic, version, planet seed, generator fingerprint. Fixed size, so a
// header that is short is a header that is broken.
constexpr size_t kHeaderBytes = 8 + 4 + 8 + 8;

// Records are written little-endian explicitly rather than by memcpying
// structs. A save file outlives the machine that wrote it, and struct layout
// does not survive a compiler flag, let alone an architecture.
void put8(std::vector<uint8_t>& b, uint8_t v) { b.push_back(v); }
void put16(std::vector<uint8_t>& b, uint16_t v) {
    b.push_back(uint8_t(v)); b.push_back(uint8_t(v >> 8));
}
void put32(std::vector<uint8_t>& b, uint32_t v) {
    for (int i = 0; i < 4; ++i) b.push_back(uint8_t(v >> (8 * i)));
}
void put64(std::vector<uint8_t>& b, uint64_t v) {
    for (int i = 0; i < 8; ++i) b.push_back(uint8_t(v >> (8 * i)));
}

uint16_t get16(const uint8_t* p) {
    return uint16_t(p[0]) | uint16_t(uint16_t(p[1]) << 8);
}
uint32_t get32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) |
           (uint32_t(p[3]) << 24);
}
uint64_t get64(const uint8_t* p) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) v |= uint64_t(p[i]) << (8 * i);
    return v;
}

// One record: a chunk address, a count, then that many (address, material)
// pairs, then a CRC32 of everything before it in the record.
//
//   u8 face | i32 x | i32 y | i32 z | u32 count | (u32, u16) * count | u32 crc
constexpr size_t kRecordFixed = 1 + 4 + 4 + 4 + 4;
constexpr size_t kPairBytes = 4 + 2;
constexpr size_t kCrcBytes = 4;

// A sane ceiling on a record's pair count. Without it a corrupt length field
// asks for a multi-gigabyte allocation before the checksum ever gets a chance
// to reject it.
constexpr uint32_t kMaxPairsPerRecord = 1u << 22;   // 4 M edits, 24 MB

bool writeAll(std::FILE* f, const std::vector<uint8_t>& bytes) {
    return bytes.empty() ||
           std::fwrite(bytes.data(), 1, bytes.size(), f) == bytes.size();
}

}  // namespace

ChunkAddress ChunkAddress::fromMetres(uint8_t face, double x, double y, double z) {
    ChunkAddress a;
    a.face = face;
    a.x = int32_t(std::floor(x / double(kChunkSize)));
    a.y = int32_t(std::floor(y / double(kChunkSize)));
    a.z = int32_t(std::floor(z / double(kChunkSize)));
    return a;
}

size_t ChunkAddressHash::operator()(const ChunkAddress& a) const {
    uint64_t h = uint64_t(a.face) * 0x9E3779B97F4A7C15ull;
    h ^= uint64_t(uint32_t(a.x)) * 0xC2B2AE3D27D4EB4Full;
    h ^= uint64_t(uint32_t(a.y)) * 0x165667B19E3779F9ull;
    h ^= uint64_t(uint32_t(a.z)) * 0x27D4EB2F165667C5ull;
    h ^= h >> 31;
    return size_t(h);
}

void EditStore::remember(const ChunkAddress& chunk, uint32_t addr, Material m) {
    chunks_[chunk][addr] = uint16_t(m);
    pending_.push_back(Pending{chunk, addr, uint16_t(m)});
}

void EditStore::setBlock(const ChunkAddress& chunk, int x, int y, int z,
                         Material m) {
    remember(chunk, VoxelAddress::wholeBlock(x, y, z).bits, m);
}

void EditStore::setMicro(const ChunkAddress& chunk, int x, int y, int z,
                         int mx, int my, int mz, Material m) {
    remember(chunk, VoxelAddress::microVoxel(x, y, z, mx, my, mz).bits, m);
}

const EditStore::ChunkEdits* EditStore::chunkEdits(
    const ChunkAddress& chunk) const {
    auto it = chunks_.find(chunk);
    return it == chunks_.end() ? nullptr : &it->second;
}

bool EditStore::lookupBlock(const ChunkAddress& chunk, int x, int y, int z,
                            Material& out) const {
    const ChunkEdits* edits = chunkEdits(chunk);
    if (!edits) return false;
    auto it = edits->find(VoxelAddress::wholeBlock(x, y, z).bits);
    if (it == edits->end()) return false;
    out = Material(it->second);
    return true;
}

bool EditStore::lookupMicro(const ChunkAddress& chunk, int x, int y, int z,
                            int mx, int my, int mz, Material& out) const {
    const ChunkEdits* edits = chunkEdits(chunk);
    if (!edits) return false;
    // A micro edit wins over a block edit on the same block: it is the finer
    // and therefore the later statement about that space.
    auto micro = edits->find(VoxelAddress::microVoxel(x, y, z, mx, my, mz).bits);
    if (micro != edits->end()) { out = Material(micro->second); return true; }
    auto block = edits->find(VoxelAddress::wholeBlock(x, y, z).bits);
    if (block != edits->end()) { out = Material(block->second); return true; }
    return false;
}

size_t EditStore::editCount() const {
    size_t n = 0;
    for (const auto& kv : chunks_) n += kv.second.size();
    return n;
}

LoadReport EditStore::load(const std::string& path, uint64_t planetSeed,
                           uint64_t generatorFingerprint) {
    chunks_.clear();
    pending_.clear();
    recordsOnDisk_ = 0;
    path_ = path;
    planetSeed_ = planetSeed;
    fingerprint_ = generatorFingerprint;

    LoadReport report;

    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        // A planet nobody has touched has no file. That is the normal case, not
        // an error, and treating it as one would mean every first visit to
        // every world logged a failure.
        report.message = "no save file yet; nothing has been changed here";
        return report;
    }
    report.fileExisted = true;

    std::vector<uint8_t> buffer;
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size > 0) {
        buffer.resize(size_t(size));
        if (std::fread(buffer.data(), 1, buffer.size(), f) != buffer.size())
            buffer.clear();
    }
    std::fclose(f);
    report.bytesRead = buffer.size();

    if (buffer.size() < kHeaderBytes) {
        report.message = "save file is too short to hold a header; ignoring it";
        return report;
    }
    if (get64(buffer.data()) != kMagic ||
        get32(buffer.data() + 8) != kFormatVersion) {
        report.message = "not an Elysium edit journal, or a newer format;"
                         " refusing to guess at it";
        return report;
    }
    report.headerValid = true;

    const uint64_t storedSeed = get64(buffer.data() + 12);
    const uint64_t storedFingerprint = get64(buffer.data() + 20);
    if (storedSeed != planetSeed) {
        report.message = "this journal belongs to a different planet;"
                         " refusing to apply it";
        return report;
    }
    if (storedFingerprint != generatorFingerprint) {
        // Loaded anyway, and flagged loudly. The edits are perfectly good data
        // about a world that no longer exists — a doorway cut into a cliff the
        // generator no longer puts there. Silently applying them would leave
        // the player's work floating in mid-air with no explanation.
        report.fingerprintMatches = false;
        report.message = "the generator has changed since this was saved;"
                         " edits may no longer line up with the terrain";
    }

    size_t offset = kHeaderBytes;
    while (offset + kRecordFixed + kCrcBytes <= buffer.size()) {
        const uint8_t* p = buffer.data() + offset;
        const uint8_t face = p[0];
        const int32_t cx = int32_t(get32(p + 1));
        const int32_t cy = int32_t(get32(p + 5));
        const int32_t cz = int32_t(get32(p + 9));
        const uint32_t count = get32(p + 13);

        if (count > kMaxPairsPerRecord) { report.truncatedTail = true; break; }
        const size_t recordBytes = kRecordFixed + size_t(count) * kPairBytes;
        if (offset + recordBytes + kCrcBytes > buffer.size()) {
            report.truncatedTail = true;
            break;
        }

        const uint32_t stored = get32(buffer.data() + offset + recordBytes);
        const uint32_t actual = uint32_t(
            crc32(0, buffer.data() + offset, uInt(recordBytes)));
        if (stored != actual) {
            // A torn write, almost certainly the last one before a crash. Stop
            // here and keep everything before it: the player loses one edit
            // rather than the planet.
            report.truncatedTail = true;
            break;
        }

        const ChunkAddress chunk{face, cx, cy, cz};
        ChunkEdits& edits = chunks_[chunk];
        const uint8_t* pairs = p + kRecordFixed;
        for (uint32_t i = 0; i < count; ++i) {
            const uint32_t addr = get32(pairs + i * kPairBytes);
            const uint16_t mat = get16(pairs + i * kPairBytes + 4);
            // Later records win. That is the whole semantics of a journal, and
            // it is why replay must be in file order.
            edits[addr] = mat;
            ++report.editsApplied;
        }
        if (edits.empty()) chunks_.erase(chunk);

        offset += recordBytes + kCrcBytes;
        ++report.recordsRead;
        ++recordsOnDisk_;
    }

    if (offset != buffer.size() && !report.truncatedTail)
        report.truncatedTail = true;

    if (report.message.empty()) {
        char buf[192];
        std::snprintf(buf, sizeof(buf),
                      "%llu edits in %zu chunks from %llu records",
                      (unsigned long long)report.editsApplied, chunks_.size(),
                      (unsigned long long)report.recordsRead);
        report.message = buf;
        if (report.truncatedTail)
            report.message += " (a torn record at the end was discarded)";
    }
    return report;
}

bool EditStore::flush(std::string* error) {
    if (pending_.empty()) return true;
    if (path_.empty()) {
        if (error) *error = "no path: load() has not been called";
        return false;
    }

    // Group the pending edits by chunk, keeping order within each chunk. One
    // record per chunk per flush rather than one per edit: a player mining a
    // tunnel touches one chunk hundreds of times, and a record header for each
    // would be most of the file.
    std::vector<ChunkAddress> order;
    std::unordered_map<ChunkAddress, std::vector<const Pending*>,
                       ChunkAddressHash> grouped;
    for (const Pending& p : pending_) {
        auto it = grouped.find(p.chunk);
        if (it == grouped.end()) {
            order.push_back(p.chunk);
            grouped[p.chunk].push_back(&p);
        } else {
            it->second.push_back(&p);
        }
    }

    const bool creating = [&] {
        std::FILE* probe = std::fopen(path_.c_str(), "rb");
        if (!probe) return true;
        std::fseek(probe, 0, SEEK_END);
        const bool empty = std::ftell(probe) < long(kHeaderBytes);
        std::fclose(probe);
        return empty;
    }();

    std::FILE* f = std::fopen(path_.c_str(), creating ? "wb" : "ab");
    if (!f) {
        if (error) *error = "cannot open " + path_ + " for writing";
        return false;
    }

    std::vector<uint8_t> out;
    if (creating) {
        put64(out, kMagic);
        put32(out, kFormatVersion);
        put64(out, planetSeed_);
        put64(out, fingerprint_);
    }

    for (const ChunkAddress& chunk : order) {
        const std::vector<const Pending*>& items = grouped[chunk];
        const size_t start = out.size();
        put8(out, chunk.face);
        put32(out, uint32_t(chunk.x));
        put32(out, uint32_t(chunk.y));
        put32(out, uint32_t(chunk.z));
        put32(out, uint32_t(items.size()));
        for (const Pending* p : items) {
            put32(out, p->addr);
            put16(out, p->material);
        }
        // The checksum covers the record and only the record, so each one is
        // independently verifiable and a torn tail cannot invalidate what came
        // before it.
        const uint32_t crc = uint32_t(
            crc32(0, out.data() + start, uInt(out.size() - start)));
        put32(out, crc);
        ++recordsOnDisk_;
    }

    const bool ok = writeAll(f, out);
    // Flushed to the OS before reporting success. Not fsync: durability against
    // a power cut would cost a disk round trip per edit batch, and the format is
    // already designed so that losing the tail costs one edit.
    if (ok) std::fflush(f);
    std::fclose(f);

    if (!ok) {
        if (error) *error = "write failed on " + path_;
        return false;
    }
    pending_.clear();
    return true;
}

bool EditStore::shouldCompact() const {
    const size_t live = editCount();
    // Twice the live count, with a floor so a small file is never churned.
    return recordsOnDisk_ > 64 && recordsOnDisk_ > live * 2;
}

bool EditStore::compact(std::string* error) {
    if (path_.empty()) {
        if (error) *error = "no path: load() has not been called";
        return false;
    }
    if (!flush(error)) return false;

    // Written to a temporary and renamed. A crash during compaction then leaves
    // the original journal untouched, which matters because compaction is the
    // one operation that touches records the player is relying on.
    const std::string temp = path_ + ".compacting";
    std::FILE* f = std::fopen(temp.c_str(), "wb");
    if (!f) {
        if (error) *error = "cannot open " + temp + " for writing";
        return false;
    }

    std::vector<uint8_t> out;
    put64(out, kMagic);
    put32(out, kFormatVersion);
    put64(out, planetSeed_);
    put64(out, fingerprint_);

    uint64_t records = 0;
    for (const auto& entry : chunks_) {
        if (entry.second.empty()) continue;
        const size_t start = out.size();
        put8(out, entry.first.face);
        put32(out, uint32_t(entry.first.x));
        put32(out, uint32_t(entry.first.y));
        put32(out, uint32_t(entry.first.z));
        put32(out, uint32_t(entry.second.size()));
        for (const auto& edit : entry.second) {
            put32(out, edit.first);
            put16(out, edit.second);
        }
        put32(out, uint32_t(crc32(0, out.data() + start,
                                  uInt(out.size() - start))));
        ++records;
    }

    const bool ok = writeAll(f, out);
    if (ok) std::fflush(f);
    std::fclose(f);
    if (!ok) {
        std::remove(temp.c_str());
        if (error) *error = "write failed on " + temp;
        return false;
    }

    std::remove(path_.c_str());
    if (std::rename(temp.c_str(), path_.c_str()) != 0) {
        if (error) *error = "could not replace " + path_;
        return false;
    }
    recordsOnDisk_ = records;
    return true;
}

}  // namespace ely
