// Intended function: Track authored/generated asset references, missing assets, duplicate stable names, material usage, LOD coverage, and packaging state.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::tools {
struct AssetAuditRecord {
    std::uint64_t recordId{};
    std::uint64_t assetId{};
    std::uint64_t category{};
    std::uint64_t status{};
    std::uint64_t referenceCount{};
    std::uint64_t flags{};
};
class AssetAuditRecordIndex {
public:
 bool upsert(AssetAuditRecord value); bool erase(std::uint64_t id); [[nodiscard]] const AssetAuditRecord* find(std::uint64_t id) const; [[nodiscard]] const std::vector<AssetAuditRecord>& rows() const noexcept { return rows_; }
private:
 static std::uint64_t keyOf(const AssetAuditRecord& value) noexcept; std::vector<AssetAuditRecord> rows_;
};
}
