// Intended function: imported render implementation for PresentationRuntime; preserves the agent-authored subsystem contract for later integration/debugging.
#pragma once

#include "render/PresentationAssets.hpp"

#include <cstdint>
#include <vector>

namespace elysium {

// Frame-local presentation admission. This is deliberately backend-neutral and
// non-authoritative: it chooses how richly already-known simulation meaning is
// presented without mutating ECS/world state.
enum class PresentationImportance : std::uint8_t {
    Critical = 0,      // warnings / direct danger / player interaction
    Strategic,         // player-built skyline, major structure, named threat
    Interactive,       // nearby machines, fauna, readable props
    Ambient,           // weather / ordinary effects
    Decorative         // regenerable detail only
};

enum class PresentationQuality : std::uint8_t { Full = 0, Reduced, Proxy, Culled };

struct PresentationCost {
    int triangles{};
    int instances{};
    int particles{};
    int lights{};
    int shadowCasters{};
    int drawCalls{};
    int effectWorkUnits{};
};

struct PresentationFrameBudget {
    int triangles{220000};
    int instances{6000};
    int particles{1200};
    int lights{32};
    int shadowCasters{768};
    int drawCalls{420};
    int effectWorkUnits{12000};
};

struct PresentationCandidate {
    std::uint64_t stableSourceId{};
    std::uint64_t contentKey{};
    PresentationImportance importance{PresentationImportance::Decorative};
    float distance{};
    PresentationCost full{};
    PresentationCost reduced{};
    PresentationCost proxy{};
    // If true, proxy representation carries player/strategic meaning and should
    // be preferred over richer lower-priority decorative candidates.
    bool preserveMeaning{};
};

struct PresentationAdmission {
    std::uint64_t stableSourceId{};
    std::uint64_t contentKey{};
    PresentationQuality quality{PresentationQuality::Culled};
    PresentationCost cost{};
};

struct PresentationFrameSelection {
    std::vector<PresentationAdmission> admissions;
    PresentationCost used{};
    int fullCount{};
    int reducedCount{};
    int proxyCount{};
    int culledCount{};
    int mandatoryProxyMisses{};
    std::uint64_t fingerprint{};
};

bool presentationCostFits(const PresentationCost& used,
                          const PresentationCost& add,
                          const PresentationFrameBudget& budget);
PresentationFrameSelection selectPresentationFrame(std::vector<PresentationCandidate> candidates,
                                                    const PresentationFrameBudget& budget);

// Helpers translate existing Agent-40 packets into runtime candidates without
// introducing renderer- or ECS-specific types.
PresentationCandidate detailPresentationCandidate(std::uint64_t stableSourceId,
                                                  std::uint64_t contentKey,
                                                  const DetailClusterPacket& packet,
                                                  float distance,
                                                  PresentationImportance importance = PresentationImportance::Decorative);
PresentationCandidate effectPresentationCandidate(std::uint64_t stableSourceId,
                                                  std::uint64_t contentKey,
                                                  const EffectPacket& packet,
                                                  float distance,
                                                  PresentationImportance importance = PresentationImportance::Ambient);
PresentationCandidate structurePresentationCandidate(std::uint64_t stableSourceId,
                                                     std::uint64_t contentKey,
                                                     int fullTriangles,
                                                     int fullDrawCalls,
                                                     int fullShadowCasters,
                                                     float distance,
                                                     PresentationImportance importance,
                                                     bool preserveMeaning = true);

struct AudioVoiceCandidate {
    std::uint64_t stableSourceId{};
    std::uint64_t cueFingerprint{};
    PresentationImportance importance{PresentationImportance::Ambient};
    float distance{};
    float gain{1.0f};
    bool loop{};
};

struct AudioVoiceSelection {
    std::vector<AudioVoiceCandidate> voices;
    int dropped{};
    std::uint64_t fingerprint{};
};

AudioVoiceSelection selectAudioVoices(std::vector<AudioVoiceCandidate> candidates, int maxVoices);

} // namespace elysium
