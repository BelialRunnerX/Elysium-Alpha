#pragma once

#include "fortress/Components.hpp"

#include <map>
#include <span>
#include <vector>

namespace elysium::fortress {

struct StockIndexEntry {
    StableId item{};
    StableId container{};
    SiteId site{};
    ContentId content;
    ContentId material;
    float quantity{};
    float reservedQuantity{};
    float contamination{};
    bool forbidden{};
};

class StockIndex {
public:
    void rebuild(std::span<const StockIndexEntry> entries);
    void upsert(StockIndexEntry entry);
    void erase(StableId item);
    [[nodiscard]] float available(std::string_view content, SiteId site) const;
    [[nodiscard]] std::vector<StableId> findAvailable(std::string_view content,
                                                      SiteId site,
                                                      float requiredQuantity,
                                                      std::size_t limit) const;

private:
    std::map<std::uint64_t, StockIndexEntry> entries_;
    std::map<std::string, std::vector<StableId>> byContent_;
};

} // namespace elysium::fortress
