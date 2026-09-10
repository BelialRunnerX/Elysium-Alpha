#pragma once

#include "fortress/Components.hpp"

#include <vector>

namespace elysium::fortress {

enum class FileFactKind : std::uint8_t {
    CitizenIdentity,
    SiteOwnership,
    Claim,
    Extraction,
    IndustrialCapacity,
    MilitaryStrength,
    ArtifactOwnership,
    Contraband,
    Route,
    Anomaly,
    Crime,
    OrganizationMembership
};

struct FileFact {
    StableId id{};
    FileFactKind kind{FileFactKind::CitizenIdentity};
    StableId subject{};
    SiteId site{};
    std::uint64_t system{};
    ContentId value;
    float numeric{};
    float confidence{};
    ContentId source;
    TimeStamp observed{};
    bool stale{};
};

struct FileKnowledge {
    OrganizationId owner{};
    std::vector<FileFact> facts;
};

void observeFileFact(FileKnowledge& file, FileFact fact);
float fileConfidence(const FileKnowledge& file, FileFactKind kind, StableId subject,
                     std::uint64_t system);
void ageFileKnowledge(FileKnowledge& file, float days);

} // namespace elysium::fortress
