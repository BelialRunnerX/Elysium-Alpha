#pragma once
#include <array>
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::combat {
using AiId=std::uint64_t;
struct AiEntityView { AiId id{}, generation{}, revision{}; std::array<double,3> position{}; bool alive{}; };
struct AiWorldSnapshot { AiId tick{}, navigationRevision{}; std::vector<AiEntityView> entities; };
enum class AiAction : std::uint8_t { Wait, Move, Attack, Ability };
enum class AiRejection : std::uint8_t { None, StaleTick, MissingActor, MissingTarget, StaleEntity, StalePath, Conflict, Invalid }; 
struct AiCandidate {
    AiId tick{}, actor{}, actorGeneration{}, actorRevision{}, target{}, targetGeneration{}, targetRevision{}, sequence{};
    AiAction action{}; std::int32_t priority{}; AiId ability{}, pathRequest{};
    std::array<double,3> destination{};
};
struct AiPathRequest { AiId id{}, tick{}, actor{}, generation{}, navigationRevision{}; std::array<double,3> from{}, to{}; };
struct AiPathResult { AiPathRequest request; bool reachable{}; std::vector<std::array<double,3>> waypoints; };
struct AiDecision { AiCandidate candidate; AiRejection rejection{}; };
struct AiCommitBatch { AiId tick{}; std::vector<AiDecision> decisions; std::vector<AiPathResult> routes; };
class AiCommandPipeline {
public:
    static bool canonicalize(AiWorldSnapshot&);
    static std::vector<AiPathRequest> pathRequests(const AiWorldSnapshot&, const std::vector<AiCandidate>&);
    static AiCommitBatch commit(const AiWorldSnapshot&, std::vector<AiCandidate>, std::vector<AiPathResult>);
    // Called after the real owner successfully applies the action. Failed Act
    // attempts are not marked, allowing explicit retry without duplicate success.
    bool acknowledge(const AiDecision&);
    bool alreadyActed(const AiCandidate&) const;
    std::vector<std::array<AiId,3>> snapshot() const;
    bool restore(const std::vector<std::array<AiId,3>>&);
private: std::map<AiId,std::pair<AiId,AiId>> lastActed_; // actor -> tick, sequence
};
}
