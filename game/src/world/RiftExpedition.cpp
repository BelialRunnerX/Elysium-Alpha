// Intended function: imported world implementation for RiftExpedition; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/RiftExpedition.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <limits>
#include <queue>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

namespace elysium {
namespace {

constexpr std::uint64_t kTopologyLabel = 0x524946545F544F50ULL;
constexpr std::uint64_t kRoomLabel = 0x524946545F524F4FULL;
constexpr std::uint64_t kRunLabel = 0x524946545F52554EULL;
constexpr std::uint64_t kBoonLabel = 0x524946545F424F4FULL;
constexpr std::uint64_t kFloorLabel = 0x524946545F464C52ULL;

class StableRng {
public:
    explicit StableRng(std::uint64_t seed) : state_(mix64(seed)) {}
    std::uint64_t next() {
        state_ = mix64(state_);
        return state_;
    }
    std::uint32_t bounded(std::uint32_t n) {
        return n == 0 ? 0 : static_cast<std::uint32_t>(next() % n);
    }
    bool newestBias() {
        // Exact integer equivalent of p=0.72 for this generator contract.
        return (next() % 100ULL) < 72ULL;
    }
private:
    std::uint64_t state_{};
};

struct Cell { int x{}; int y{}; };

int indexOf(int x, int y) { return y * RiftTopology::GridSize + x; }
bool inBounds(int x, int y) { return x >= 0 && y >= 0 && x < RiftTopology::GridSize && y < RiftTopology::GridSize; }

constexpr std::array<int, 4> kDx{0, 1, 0, -1};
constexpr std::array<int, 4> kDy{-1, 0, 1, 0};
constexpr std::array<std::uint8_t, 4> kDoorBit{RiftDoorNorth, RiftDoorEast, RiftDoorSouth, RiftDoorWest};

std::uint8_t doorMaskFor(const std::array<bool, RiftTopology::GridSize * RiftTopology::GridSize>& occupied,
                         int x, int y) {
    std::uint8_t mask = RiftDoorNone;
    for (int d = 0; d < 4; ++d) {
        const int nx = x + kDx[d], ny = y + kDy[d];
        if (inBounds(nx, ny) && occupied[static_cast<std::size_t>(indexOf(nx, ny))]) mask |= kDoorBit[d];
    }
    return mask;
}

std::vector<int> bfsDepths(const std::array<bool, RiftTopology::GridSize * RiftTopology::GridSize>& occupied) {
    std::vector<int> depth(RiftTopology::GridSize * RiftTopology::GridSize, -1);
    constexpr int c = RiftTopology::GridSize / 2;
    std::queue<Cell> q;
    depth[indexOf(c, c)] = 0;
    q.push({c, c});
    while (!q.empty()) {
        const auto cell = q.front(); q.pop();
        const int base = depth[indexOf(cell.x, cell.y)];
        for (int d = 0; d < 4; ++d) {
            const int nx = cell.x + kDx[d], ny = cell.y + kDy[d];
            if (!inBounds(nx, ny)) continue;
            const auto ni = static_cast<std::size_t>(indexOf(nx, ny));
            if (!occupied[ni] || depth[ni] >= 0) continue;
            depth[ni] = base + 1;
            q.push({nx, ny});
        }
    }
    return depth;
}

struct LayoutBuild {
    std::array<bool, RiftTopology::GridSize * RiftTopology::GridSize> occupied{};
    std::vector<Cell> creationOrder;
};

LayoutBuild buildBiased(std::uint64_t seed) {
    LayoutBuild out;
    constexpr int c = RiftTopology::GridSize / 2;
    out.occupied[indexOf(c, c)] = true;
    out.creationOrder.push_back({c, c});
    StableRng rng(seed);
    const int maxAttempts = RiftTopology::RoomCount * 200;
    for (int attempt = 0; attempt < maxAttempts && static_cast<int>(out.creationOrder.size()) < RiftTopology::RoomCount; ++attempt) {
        const std::size_t sourceIndex = rng.newestBias()
            ? out.creationOrder.size() - 1
            : static_cast<std::size_t>(rng.bounded(static_cast<std::uint32_t>(out.creationOrder.size())));
        const Cell source = out.creationOrder[sourceIndex];
        const int d = static_cast<int>(rng.bounded(4));
        const int nx = source.x + kDx[d], ny = source.y + kDy[d];
        if (!inBounds(nx, ny)) continue;
        const auto ni = static_cast<std::size_t>(indexOf(nx, ny));
        if (out.occupied[ni]) continue;
        out.occupied[ni] = true;
        out.creationOrder.push_back({nx, ny});
    }
    return out;
}

LayoutBuild fallbackLayout() {
    // Connected 12-room shape with three+ non-entrance leaves. This is used only
    // if bounded biased attempts fail to produce a valid layout; it guarantees
    // the public generator contract without an unbounded retry loop.
    LayoutBuild out;
    constexpr std::array<Cell, RiftTopology::RoomCount> cells{{
        {4,4},{4,3},{4,2},{4,1},{5,3},{6,3},{3,3},{2,3},{4,5},{4,6},{5,5},{3,5}
    }};
    for (const auto c : cells) {
        out.occupied[static_cast<std::size_t>(indexOf(c.x,c.y))] = true;
        out.creationOrder.push_back(c);
    }
    return out;
}

bool layoutHasRequiredLeaves(const LayoutBuild& layout) {
    if (static_cast<int>(layout.creationOrder.size()) != RiftTopology::RoomCount) return false;
    const auto depth = bfsDepths(layout.occupied);
    int maxDepth = -1;
    Cell boss{};
    for (const auto c : layout.creationOrder) {
        const int dep = depth[indexOf(c.x,c.y)];
        if (dep < 0) return false;
        const auto key = std::tuple{dep, -indexOf(c.x,c.y)};
        const auto bossKey = std::tuple{maxDepth, -indexOf(boss.x,boss.y)};
        if (key > bossKey) { maxDepth = dep; boss = c; }
    }
    int lootLeaves = 0;
    for (const auto c : layout.creationOrder) {
        if (c.x == 4 && c.y == 4) continue;
        if (c.x == boss.x && c.y == boss.y) continue;
        if (std::popcount(static_cast<unsigned>(doorMaskFor(layout.occupied,c.x,c.y))) == 1) ++lootLeaves;
    }
    return lootLeaves >= RiftTopology::LootRoomCount;
}

std::uint64_t roomStableId(std::uint64_t seed, int x, int y) {
    auto id = mix64(seed ^ kRoomLabel ^ static_cast<std::uint64_t>(indexOf(x,y)));
    return id == 0 ? 1 : id;
}

bool validBoonInt(int v) { return v >= 0 && v <= static_cast<int>(RiftBoon::ArtifactSense); }

} // namespace

RiftTopology RiftTopology::generate(std::uint64_t seed) {
    LayoutBuild layout;
    bool found = false;
    // Retrying with a labeled deterministic salt preserves the documented biased
    // growth while guaranteeing two distinct non-boss dead-end loot rooms.
    for (std::uint64_t attempt = 0; attempt < 256; ++attempt) {
        layout = buildBiased(mix64(seed ^ kTopologyLabel ^ attempt));
        if (layoutHasRequiredLeaves(layout)) { found = true; break; }
    }
    if (!found) layout = fallbackLayout();

    const auto depth = bfsDepths(layout.occupied);
    Cell boss{4,4};
    int maxDepth = -1;
    for (const auto c : layout.creationOrder) {
        const int dep = depth[indexOf(c.x,c.y)];
        const auto candidateKey = std::tuple{dep, -indexOf(c.x,c.y)};
        const auto bossKey = std::tuple{maxDepth, -indexOf(boss.x,boss.y)};
        if (candidateKey > bossKey) { maxDepth = dep; boss = c; }
    }

    std::vector<Cell> leaves;
    for (const auto c : layout.creationOrder) {
        if ((c.x == 4 && c.y == 4) || (c.x == boss.x && c.y == boss.y)) continue;
        if (std::popcount(static_cast<unsigned>(doorMaskFor(layout.occupied,c.x,c.y))) == 1) leaves.push_back(c);
    }
    std::sort(leaves.begin(), leaves.end(), [&](const Cell& a, const Cell& b) {
        const auto ka = std::tuple{depth[indexOf(a.x,a.y)], -indexOf(a.x,a.y)};
        const auto kb = std::tuple{depth[indexOf(b.x,b.y)], -indexOf(b.x,b.y)};
        return ka > kb;
    });

    std::set<int> lootIndices;
    for (int i = 0; i < LootRoomCount && i < static_cast<int>(leaves.size()); ++i)
        lootIndices.insert(indexOf(leaves[static_cast<std::size_t>(i)].x, leaves[static_cast<std::size_t>(i)].y));

    RiftTopology out;
    out.seed_ = seed;
    out.rooms_.reserve(RoomCount);
    for (const auto c : layout.creationOrder) {
        RiftRoom room{};
        room.stableId = roomStableId(seed,c.x,c.y);
        room.x = static_cast<std::uint8_t>(c.x);
        room.y = static_cast<std::uint8_t>(c.y);
        room.depth = static_cast<std::uint8_t>(std::max(0, depth[indexOf(c.x,c.y)]));
        room.doors = doorMaskFor(layout.occupied,c.x,c.y);
        if (c.x == 4 && c.y == 4) room.kind = RiftRoomKind::Entrance;
        else if (c.x == boss.x && c.y == boss.y) room.kind = RiftRoomKind::Boss;
        else if (lootIndices.contains(indexOf(c.x,c.y))) room.kind = RiftRoomKind::Loot;
        else room.kind = RiftRoomKind::Filler;
        out.rooms_.push_back(room);
    }
    std::sort(out.rooms_.begin(), out.rooms_.end(), [](const auto& a, const auto& b) {
        return std::tie(a.y,a.x,a.stableId) < std::tie(b.y,b.x,b.stableId);
    });
    return out;
}

const RiftRoom* RiftTopology::roomAt(int x, int y) const {
    const auto it = std::find_if(rooms_.begin(), rooms_.end(), [=](const RiftRoom& r){ return r.x == x && r.y == y; });
    return it == rooms_.end() ? nullptr : &*it;
}

RiftValidation RiftTopology::validate() const {
    if (seed_ == 0 && rooms_.empty()) return {false,"empty topology"};
    if (static_cast<int>(rooms_.size()) != RoomCount) return {false,"room count is not 12"};
    std::array<bool, GridSize * GridSize> occupied{};
    int entranceCount=0,bossCount=0,lootCount=0,maxDepth=-1;
    const RiftRoom* boss=nullptr;
    for (const auto& room : rooms_) {
        if (room.x >= GridSize || room.y >= GridSize || room.stableId == 0) return {false,"invalid room coordinate/id"};
        const auto i=static_cast<std::size_t>(indexOf(room.x,room.y));
        if (occupied[i]) return {false,"duplicate room coordinate"};
        occupied[i]=true;
        maxDepth=std::max(maxDepth,static_cast<int>(room.depth));
        if (room.kind==RiftRoomKind::Entrance) ++entranceCount;
        else if (room.kind==RiftRoomKind::Boss) {++bossCount;boss=&room;}
        else if (room.kind==RiftRoomKind::Loot) ++lootCount;
    }
    const auto* entrance=roomAt(GridSize/2,GridSize/2);
    if (!entrance || entrance->kind!=RiftRoomKind::Entrance || entrance->depth!=0 || entranceCount!=1) return {false,"entrance contract failed"};
    if (bossCount!=1 || !boss || boss->depth!=maxDepth) return {false,"boss is not unique/max-depth"};
    if (lootCount!=LootRoomCount) return {false,"loot room count is not two"};

    // Recompute BFS and door symmetry rather than trusting stored depth/masks.
    const auto recomputed=bfsDepths(occupied);
    for (const auto& room:rooms_) {
        if (recomputed[indexOf(room.x,room.y)]<0 || recomputed[indexOf(room.x,room.y)]!=room.depth) return {false,"BFS depth mismatch/unreachable room"};
        const auto expected=doorMaskFor(occupied,room.x,room.y);
        if (room.doors!=expected) return {false,"door symmetry/mask mismatch"};
        if (room.kind==RiftRoomKind::Loot && std::popcount(static_cast<unsigned>(room.doors))!=1) return {false,"loot room is not a dead end"};
    }
    return {true,{}};
}

std::uint64_t RiftTopology::fingerprint() const {
    std::uint64_t h=mix64(seed_^GeneratorFingerprint);
    for (const auto& room:rooms_) {
        h=mix64(h^room.stableId);
        h=mix64(h^(static_cast<std::uint64_t>(room.x)<<0U)^(static_cast<std::uint64_t>(room.y)<<8U)^
                (static_cast<std::uint64_t>(room.depth)<<16U)^(static_cast<std::uint64_t>(room.doors)<<24U)^
                (static_cast<std::uint64_t>(room.kind)<<32U));
    }
    return h;
}

RiftRunState RiftRunState::begin(std::uint64_t siteId, std::uint64_t runSeed, int openerLevel, PlanetClass theme) {
    RiftRunState run;
    run.siteId_=siteId==0?1:siteId;
    run.runSeed_=runSeed==0?mix64(run.siteId_^kRunLabel):runSeed;
    run.runId_=mix64(run.runSeed_^run.siteId_^kRunLabel);
    if (run.runId_==0) run.runId_=1;
    run.openerLevel_=std::max(1,openerLevel);
    run.theme_=theme;
    run.active_=true;
    return run;
}

std::uint64_t RiftRunState::floorSeed() const {
    return mix64(runSeed_^kFloorLabel^static_cast<std::uint64_t>(depth_));
}

RiftTopology RiftRunState::topology() const { return RiftTopology::generate(floorSeed()); }

RiftScalingContext RiftRunState::scaling() const {
    RiftScalingContext out{};
    out.depth=depth_;
    out.openerLevel=openerLevel_;
    out.depthDominantRank=depth_*16+std::clamp(openerLevel_-1,0,15);
    return out;
}

bool RiftRunState::addLoot(int value, int heatAdded) {
    if (!active_ || value<0 || heatAdded<0) return false;
    if (value>std::numeric_limits<int>::max()-carriedValue_ || heatAdded>std::numeric_limits<int>::max()-heat_) return false;
    carriedValue_+=value;
    heat_+=heatAdded;
    return true;
}

bool RiftRunState::descend(bool bossDefeated) {
    if (!active_ || !bossDefeated || depth_==std::numeric_limits<int>::max()) return false;
    ++depth_;
    return true;
}

std::array<RiftBoon,3> RiftRunState::boonOffer() const {
    constexpr int count=static_cast<int>(RiftBoon::ArtifactSense)+1;
    std::array<RiftBoon,3> out{};
    std::set<int> used;
    StableRng rng(mix64(runId_^kBoonLabel^static_cast<std::uint64_t>(depth_)));
    for (int i=0;i<3;++i) {
        int candidate=static_cast<int>(rng.bounded(count));
        for (int guard=0;guard<count && used.contains(candidate);++guard) candidate=(candidate+1)%count;
        used.insert(candidate);
        out[static_cast<std::size_t>(i)]=static_cast<RiftBoon>(candidate);
    }
    return out;
}

bool RiftRunState::acceptBoon(RiftBoon boon) {
    if (!active_) return false;
    const int raw=static_cast<int>(boon);
    if (!validBoonInt(raw)) return false;
    const auto offer=boonOffer();
    if (std::find(offer.begin(),offer.end(),boon)==offer.end()) return false;
    if (std::find(boons_.begin(),boons_.end(),boon)!=boons_.end()) return false;
    boons_.push_back(boon);
    std::sort(boons_.begin(),boons_.end());
    return true;
}

void RiftRunState::clearTransientRunState() {
    depth_=1;
    carriedValue_=0;
    heat_=0;
    boons_.clear();
    active_=false;
}

RiftRunResolution RiftRunState::bankAndExit() {
    RiftRunResolution result{};
    if (!active_) return result;
    result.bankedGained=carriedValue_;
    result.persistentSuspicionHeat=heat_/PersistentHeatDivisor;
    if (carriedValue_<=std::numeric_limits<int>::max()-bankedValue_) bankedValue_+=carriedValue_;
    else bankedValue_=std::numeric_limits<int>::max();
    result.ejected=false;
    clearTransientRunState();
    return result;
}

RiftRunResolution RiftRunState::die() {
    RiftRunResolution result{};
    if (!active_) return result;
    result.carriedLost=carriedValue_;
    // Death collapses the instance and loses unbanked run loot. In-run heat is
    // not rolled into the persistent meter here; the design specifies partial
    // roll-in on exit, and the caller can deliberately revise this later.
    result.persistentSuspicionHeat=0;
    result.ejected=true;
    clearTransientRunState();
    return result;
}

void RiftRunState::restart() {
    if (restartSequence_!=std::numeric_limits<std::uint64_t>::max()) ++restartSequence_;
    runId_=mix64(runSeed_^siteId_^kRunLabel^restartSequence_);
    if (runId_==0) runId_=1;
    depth_=1; carriedValue_=0; heat_=0; boons_.clear(); active_=true;
}

std::string RiftRunState::serialize() const {
    std::ostringstream out;
    out << "ELYSIUM_RIFT_RUN " << StateVersion << ' ' << siteId_ << ' ' << runId_ << ' ' << runSeed_ << ' ' << restartSequence_ << ' '
        << depth_ << ' ' << openerLevel_ << ' ' << static_cast<int>(theme_) << ' ' << carriedValue_ << ' ' << bankedValue_ << ' '
        << heat_ << ' ' << (active_?1:0) << ' ' << boons_.size() << '\n';
    for (auto boon:boons_) out << "boon " << static_cast<int>(boon) << '\n';
    return out.str();
}

bool RiftRunState::restore(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string magic; int version=0,theme=0,active=0; std::size_t boonCount=0;
    RiftRunState parsed;
    if (!(in>>magic>>version>>parsed.siteId_>>parsed.runId_>>parsed.runSeed_>>parsed.restartSequence_>>parsed.depth_>>parsed.openerLevel_>>theme>>
          parsed.carriedValue_>>parsed.bankedValue_>>parsed.heat_>>active>>boonCount) ||
        magic!="ELYSIUM_RIFT_RUN" || version!=StateVersion || parsed.siteId_==0 || parsed.runId_==0 || parsed.runSeed_==0 ||
        parsed.depth_<1 || parsed.openerLevel_<1 || theme<0 || theme>255 || parsed.carriedValue_<0 || parsed.bankedValue_<0 || parsed.heat_<0 ||
        (active!=0 && active!=1) || boonCount>static_cast<std::size_t>(static_cast<int>(RiftBoon::ArtifactSense)+1)) {
        if(error)*error="invalid rift run header/state";
        return false;
    }
    parsed.theme_=static_cast<PlanetClass>(theme);
    parsed.active_=active!=0;
    std::set<int> seen;
    std::string tag;
    for(std::size_t i=0;i<boonCount;++i) {
        int raw=-1;
        if(!(in>>tag>>raw) || tag!="boon" || !validBoonInt(raw) || !seen.insert(raw).second) {
            if(error)*error="invalid/duplicate rift boon";
            return false;
        }
        parsed.boons_.push_back(static_cast<RiftBoon>(raw));
    }
    if(in>>tag) { if(error)*error="trailing rift run data"; return false; }
    std::sort(parsed.boons_.begin(),parsed.boons_.end());
    *this=std::move(parsed);
    return true;
}

const char* riftBoonName(RiftBoon boon) {
    static constexpr std::array<const char*,12> names={
        "Void Echo","Plasma Wake","Neural Thread","Dimensional Step","Kinetic Anchor","Salvager Instinct",
        "Second Wind","Hunter Mark","Quiet File","Deep Scanner","Emergency Seal","Artifact Sense"
    };
    const auto i=static_cast<std::size_t>(boon);
    return i<names.size()?names[i]:"Unknown Boon";
}

} // namespace elysium
