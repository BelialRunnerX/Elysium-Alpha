#pragma once

#include "fortress/Components.hpp"

#include <map>
#include <span>
#include <unordered_map>
#include <vector>

namespace elysium::fortress {

struct JobBoardEntry {
    JobId id{};
    ContentId labor;
    PriorityBand priority{PriorityBand::Normal};
    SiteId site{};
    StableId worker{};
    JobState state{JobState::Pending};
};

class JobBoard {
public:
    void upsert(JobBoardEntry entry);
    void erase(JobId id);
    [[nodiscard]] const JobBoardEntry* find(JobId id) const;
    [[nodiscard]] std::vector<JobId> candidates(std::string_view labor,
                                                PriorityBand minimumPriority,
                                                SiteId site,
                                                std::size_t limit) const;
    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }

private:
    std::unordered_map<std::uint64_t, JobBoardEntry> entries_;
    std::map<std::string, std::vector<JobId>> byLabor_;
};

} // namespace elysium::fortress
