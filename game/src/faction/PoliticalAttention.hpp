#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <vector>
namespace elysium::faction {
enum class AttentionKind : std::uint8_t { EmpireKill, UnswornKill, OrdinaryHostileKill, RichOre,
    OrdinaryOre, Socket, Reforge, Ascension, ClaimFiled, ClaimDiscovered, Extractor, Reactor, Relay, Count };
enum class AttentionReason { Applied, Duplicate, ProvenanceDenied, ChanceMiss, MissingRule, Invalid };
struct AttentionRule { AttentionKind kind{}; double favor{}, suspicion{}, chance{1}; bool naturalSourceRequired{}; };
struct AttentionEvent {
    std::uint64_t id{}, tick{}, player{}, file{}, system{}, source{}; AttentionKind kind{};
    double passiveScale{1}, presenceScale{1}; bool playerPlaced{}, selfGenerated{}, naturalSource{};
};
struct SystemAttention { std::uint64_t system{}; double pressure{}, floor{}; };
struct PoliticalFile {
    std::uint64_t player{}, file{}; double favor{};
    std::vector<SystemAttention> systems;
};
struct AttentionChange {
    AttentionEvent event; AttentionReason reason{}; double favorBefore{},favorAfter{},suspicionBefore{},suspicionAfter{};
    bool activeBefore{},activeAfter{};
};
struct PoliticalSnapshot { std::vector<PoliticalFile> files; std::vector<std::uint64_t> consumed; };
class PoliticalAttention {
public:
    bool publish(AttentionRule);
    std::vector<AttentionChange> applyBatch(std::vector<AttentionEvent>);
    // Input comes from the authoritative square-root claim policy query. This
    // ledger does not replace it with area/count arithmetic.
    bool setClaimFloor(std::uint64_t player,std::uint64_t file,std::uint64_t system,double queriedFloor);
    bool decay(double seconds,double pressurePerSecond,double favorPerSecond);
    double favor(std::uint64_t player,std::uint64_t file) const;
    double suspicion(std::uint64_t player,std::uint64_t file,std::uint64_t system) const;
    PoliticalSnapshot snapshot() const;
    bool restore(const PoliticalSnapshot&);
private:
    struct FileState { double favor{};std::map<std::uint64_t,SystemAttention> systems; };
    using Key=std::pair<std::uint64_t,std::uint64_t>;
    std::map<Key,FileState> files_;std::map<AttentionKind,AttentionRule> rules_;
    std::set<std::uint64_t> consumed_;
};
}
