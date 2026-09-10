// Intended function: imported world implementation for Derelict; preserves the agent-authored subsystem contract for later integration/debugging.
#include "world/Derelict.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>
#include <queue>
#include <sstream>
#include <tuple>
#include <utility>

namespace elysium {
namespace {
constexpr std::uint64_t kSectionLabel = 0x4445525F53454354ULL;
constexpr std::uint64_t kDoorLabel = 0x4445525F444F4F52ULL;

float unit(std::uint64_t h) {
    return static_cast<float>((h >> 40U) & 0xFFFFFFULL) / static_cast<float>(0xFFFFFFULL);
}

bool validUnit(float value) { return std::isfinite(value) && value >= 0.0f && value <= 1.0f; }
}

DerelictState DerelictState::generate(const SiteDescriptor& site, int sectionCount) {
    DerelictState d;
    d.siteId_ = site.stableId;
    sectionCount = std::clamp(sectionCount, 2, 32);
    d.sections_.reserve(static_cast<std::size_t>(sectionCount));
    for (int i = 0; i < sectionCount; ++i) {
        const auto h = mix64(site.stableId ^ kSectionLabel ^ static_cast<std::uint64_t>(i));
        DerelictSection s{};
        s.index = static_cast<std::uint8_t>(i);
        s.stableId = mix64(h ^ 0x53454354494F4EULL);
        if (s.stableId == 0) s.stableId = static_cast<std::uint64_t>(i + 1);
        s.powered = i == 0 ? false : ((h >> 2U) & 0x03ULL) == 0;
        s.pressure = ((h >> 5U) & 0x03ULL) == 0 ? 0.0f : unit(h ^ 0x5052455353555245ULL);
        s.gravity = ((h >> 9U) & 0x03ULL) == 0 ? 0.0f : (0.35f + 0.65f * unit(h ^ 0x47524156495459ULL));
        s.fire = ((h >> 12U) & 0x07ULL) == 0 ? (0.25f + 0.75f * unit(h ^ 0x46495245ULL)) : 0.0f;
        s.contamination = ((h >> 15U) & 0x07ULL) <= 1 ? (0.2f + 0.8f * unit(h ^ 0x434F4E54414DULL)) : 0.0f;
        s.security = static_cast<DerelictSecurityState>((h >> 19U) & 0x03ULL);
        s.bulkheadOpen = ((h >> 21U) & 0x03ULL) != 0;
        s.salvageMass = 20 + static_cast<int>((h >> 24U) % 181ULL);
        s.historicalCrewId = mix64(site.stableId ^ 0x43524557ULL ^ static_cast<std::uint64_t>(i));
        s.historicalOwnerId = site.historicalOwnerId;
        d.sections_.push_back(s);
    }

    // Authored-kit semantics are represented as a readable spine plus a small
    // number of deterministic cross-links. No arbitrary noise-room graph.
    for (int i = 0; i + 1 < sectionCount; ++i) {
        DerelictLink link{};
        link.a = static_cast<std::uint8_t>(i);
        link.b = static_cast<std::uint8_t>(i + 1);
        link.stableDoorId = mix64(site.stableId ^ kDoorLabel ^ static_cast<std::uint64_t>(i));
        d.links_.push_back(link);
        d.doorOpen_[link.stableDoorId] = d.sections_[static_cast<std::size_t>(i + 1)].bulkheadOpen;
    }
    for (int i = 0; i + 3 < sectionCount; i += 3) {
        const auto h = mix64(site.stableId ^ 0x43524F53534C494EULL ^ static_cast<std::uint64_t>(i));
        if ((h & 1ULL) == 0) continue;
        DerelictLink link{};
        link.a = static_cast<std::uint8_t>(i);
        link.b = static_cast<std::uint8_t>(i + 3);
        link.stableDoorId = mix64(h ^ kDoorLabel);
        d.links_.push_back(link);
        d.doorOpen_[link.stableDoorId] = ((h >> 8U) & 1ULL) != 0;
    }
    std::sort(d.links_.begin(), d.links_.end(), [](const auto& a, const auto& b) {
        return std::tie(a.a, a.b, a.stableDoorId) < std::tie(b.a, b.b, b.stableDoorId);
    });
    return d;
}

DerelictSection* DerelictState::section(std::uint8_t index) {
    return index < sections_.size() ? &sections_[index] : nullptr;
}
const DerelictSection* DerelictState::section(std::uint8_t index) const {
    return index < sections_.size() ? &sections_[index] : nullptr;
}

bool DerelictState::setPowered(std::uint8_t index, bool powered) {
    auto* s = section(index); if (!s) return false;
    s->powered = powered;
    if (powered && s->security == DerelictSecurityState::Offline) s->security = DerelictSecurityState::Dormant;
    return true;
}

bool DerelictState::setBulkhead(std::uint8_t a, std::uint8_t b, bool open) {
    for (const auto& link : links_) {
        if (!((link.a == a && link.b == b) || (link.a == b && link.b == a))) continue;
        doorOpen_[link.stableDoorId] = open;
        return true;
    }
    return false;
}

bool DerelictState::emergencySeal(std::uint8_t index, bool sealed) {
    auto* s = section(index); if (!s) return false;
    s->emergencySeal = sealed;
    if (sealed && s->pressure < 0.05f) s->pressure = 0.05f;
    return true;
}

int DerelictState::salvage(std::uint8_t index, int requestedMass) {
    auto* s = section(index); if (!s || requestedMass <= 0) return 0;
    const int take = std::min(requestedMass, s->remainingSalvageMass());
    s->salvagedMass += take;
    return take;
}

void DerelictState::update(float dt) {
    if (!std::isfinite(dt) || dt <= 0.0f) return;
    dt = std::min(dt, 10.0f);
    for (auto& s : sections_) {
        if (s.fire > 0.0f) {
            const float oxygenFactor = std::clamp(s.pressure, 0.0f, 1.0f);
            s.fire = std::max(0.0f, s.fire - dt * (0.02f + (s.emergencySeal ? 0.08f : 0.0f)));
            s.pressure = std::max(0.0f, s.pressure - dt * 0.02f * s.fire * oxygenFactor);
            s.contamination = std::min(1.0f, s.contamination + dt * 0.01f * s.fire);
        }
        if (s.powered && s.emergencySeal && s.pressure < 1.0f) s.pressure = std::min(1.0f, s.pressure + dt * 0.04f);
    }
}

std::uint32_t DerelictState::hazards(std::uint8_t index) const {
    const auto* s = section(index); if (!s) return DerelictHazardNone;
    std::uint32_t h = DerelictHazardNone;
    if (s->pressure < 0.35f) h |= DerelictHazardVacuum;
    if (s->fire > 0.1f) h |= DerelictHazardFire;
    if (s->contamination > 0.1f) h |= DerelictHazardContamination;
    if (s->contamination > 0.75f) h |= DerelictHazardRadiation;
    if (s->gravity < 0.2f) h |= DerelictHazardZeroGravity;
    if (s->security == DerelictSecurityState::Alert || s->security == DerelictSecurityState::LockedDown) h |= DerelictHazardSecurity;
    return h;
}

bool DerelictState::passable(std::uint8_t from, std::uint8_t to, const DerelictTraversalPolicy& policy) const {
    const auto* target = section(to); if (!target) return false;
    bool linked = false;
    for (const auto& link : links_) {
        if (!((link.a == from && link.b == to) || (link.a == to && link.b == from))) continue;
        linked = true;
        const auto it = doorOpen_.find(link.stableDoorId);
        if (it == doorOpen_.end() || !it->second) return false;
        break;
    }
    if (!linked) return false;
    const auto h = hazards(to);
    if ((h & DerelictHazardVacuum) && !policy.vacuumProtection) return false;
    if ((h & DerelictHazardFire) && !policy.fireProtection) return false;
    if ((h & DerelictHazardContamination) && !policy.contaminationProtection) return false;
    if ((h & DerelictHazardRadiation) && !policy.radiationProtection) return false;
    if ((h & DerelictHazardZeroGravity) && !policy.zeroGravityMobility) return false;
    if ((h & DerelictHazardSecurity) && !policy.securityAccess) return false;
    return true;
}

std::vector<std::uint8_t> DerelictState::reachable(std::uint8_t start, const DerelictTraversalPolicy& policy) const {
    std::vector<std::uint8_t> out;
    if (!section(start)) return out;
    std::vector<bool> visited(sections_.size(), false);
    std::queue<std::uint8_t> q;
    visited[start] = true; q.push(start);
    while (!q.empty()) {
        const auto current = q.front(); q.pop(); out.push_back(current);
        for (const auto& link : links_) {
            std::optional<std::uint8_t> next;
            if (link.a == current) next = link.b;
            else if (link.b == current) next = link.a;
            if (!next || visited[*next] || !passable(current, *next, policy)) continue;
            visited[*next] = true; q.push(*next);
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

DerelictInspection DerelictState::inspect() const {
    DerelictInspection i{}; i.siteId = siteId_; i.sections = static_cast<int>(sections_.size());
    for (const auto& s : sections_) {
        i.poweredSections += s.powered ? 1 : 0;
        i.pressurizedSections += s.pressure >= 0.55f ? 1 : 0;
        i.burningSections += s.fire > 0.1f ? 1 : 0;
        i.contaminatedSections += s.contamination > 0.1f ? 1 : 0;
        i.lockedSections += s.security == DerelictSecurityState::LockedDown ? 1 : 0;
        i.salvageMassRemaining += s.remainingSalvageMass();
    }
    return i;
}

std::string DerelictState::serialize() const {
    std::ostringstream out;
    out << std::setprecision(9);
    out << "ELYSIUM_DERELICT 1 " << siteId_ << ' ' << sections_.size() << ' ' << links_.size() << '\n';
    for (const auto& s : sections_) {
        out << "section " << s.stableId << ' ' << static_cast<int>(s.index) << ' ' << (s.powered?1:0) << ' '
            << s.pressure << ' ' << s.gravity << ' ' << s.fire << ' ' << s.contamination << ' '
            << static_cast<int>(s.security) << ' ' << (s.bulkheadOpen?1:0) << ' ' << (s.emergencySeal?1:0) << ' '
            << s.salvageMass << ' ' << s.salvagedMass << ' ' << s.historicalCrewId << ' ' << s.historicalOwnerId << '\n';
    }
    for (const auto& link : links_) {
        const auto it = doorOpen_.find(link.stableDoorId);
        out << "link " << static_cast<int>(link.a) << ' ' << static_cast<int>(link.b) << ' ' << link.stableDoorId << ' '
            << (it != doorOpen_.end() && it->second ? 1 : 0) << '\n';
    }
    return out.str();
}

bool DerelictState::restore(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string magic; int version; std::uint64_t siteId; std::size_t sectionCount, linkCount;
    if (!(in >> magic >> version >> siteId >> sectionCount >> linkCount) || magic != "ELYSIUM_DERELICT" || version != 1 || siteId == 0 || sectionCount < 2 || sectionCount > 32 || linkCount > 128) {
        if (error) *error = "invalid derelict header";
        return false;
    }
    std::vector<DerelictSection> sections;
    std::vector<DerelictLink> links;
    std::map<std::uint64_t, bool> doors;
    std::string tag;
    for (std::size_t n = 0; n < sectionCount; ++n) {
        if (!(in >> tag) || tag != "section") { if (error) *error = "missing derelict section"; return false; }
        DerelictSection s{}; int index, powered, security, bulkhead, seal;
        if (!(in >> s.stableId >> index >> powered >> s.pressure >> s.gravity >> s.fire >> s.contamination >> security >> bulkhead >> seal >>
              s.salvageMass >> s.salvagedMass >> s.historicalCrewId >> s.historicalOwnerId) || s.stableId == 0 || index < 0 || index >= static_cast<int>(sectionCount) ||
            (powered!=0 && powered!=1) || security < 0 || security > 3 || (bulkhead!=0 && bulkhead!=1) || (seal!=0 && seal!=1) ||
            !validUnit(s.pressure) || !validUnit(s.gravity) || !validUnit(s.fire) || !validUnit(s.contamination) || s.salvageMass < 0 || s.salvagedMass < 0 || s.salvagedMass > s.salvageMass) {
            if (error) *error = "invalid derelict section record";
            return false;
        }
        s.index=static_cast<std::uint8_t>(index); s.powered=powered!=0; s.security=static_cast<DerelictSecurityState>(security); s.bulkheadOpen=bulkhead!=0; s.emergencySeal=seal!=0;
        sections.push_back(s);
    }
    std::sort(sections.begin(), sections.end(), [](const auto& a,const auto& b){return a.index<b.index;});
    for (std::size_t i=0;i<sections.size();++i) if (sections[i].index != i) { if(error)*error="derelict section index gap"; return false; }
    for (std::size_t n = 0; n < linkCount; ++n) {
        if (!(in >> tag) || tag != "link") { if (error) *error = "missing derelict link"; return false; }
        int a,b,open; DerelictLink link{};
        if (!(in >> a >> b >> link.stableDoorId >> open) || a < 0 || b < 0 || a >= static_cast<int>(sectionCount) || b >= static_cast<int>(sectionCount) || a == b ||
            link.stableDoorId == 0 || (open!=0 && open!=1)) { if (error) *error = "invalid derelict link"; return false; }
        link.a=static_cast<std::uint8_t>(a); link.b=static_cast<std::uint8_t>(b);
        if (doors.contains(link.stableDoorId)) { if(error)*error="duplicate derelict door id"; return false; }
        doors[link.stableDoorId]=open!=0; links.push_back(link);
    }
    if (in >> tag) { if(error)*error="trailing derelict data"; return false; }
    siteId_=siteId; sections_=std::move(sections); links_=std::move(links); doorOpen_=std::move(doors);
    return true;
}

} // namespace elysium
