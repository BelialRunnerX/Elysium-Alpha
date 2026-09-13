#include "foundation/TerminologyManifest.hpp"
#include <algorithm>
#include <sstream>

namespace elysium::terms {

bool Manifest::add(Term term) {
    if (!term.id || term.preferred.empty() || term.definition.empty() || term.owner.empty() || terms_.contains(term.id)) return false;
    terms_[term.id] = std::move(term);
    return true;
}

std::optional<Term> Manifest::find(TermId id) const {
    auto it = terms_.find(id);
    return it == terms_.end() ? std::nullopt : std::optional<Term>{it->second};
}

std::vector<Issue> Manifest::audit() const {
    std::vector<Issue> out;
    std::map<std::string, TermId> names;
    std::map<std::string, TermId> keys;
    for (const auto& [id, term] : terms_) {
        auto addName = [&](const std::string& name) {
            if (name.empty()) return;
            auto [it, inserted] = names.emplace(name, id);
            if (!inserted && it->second != id) out.push_back({id, "conflicting_alias_or_term"});
        };
        addName(term.preferred);
        for (const auto& alias : term.aliases) addName(alias);
        for (const auto& alias : term.legacyAliases) addName(alias);
        if (!term.serializedKey.empty()) {
            auto [it, inserted] = keys.emplace(term.serializedKey, id);
            if (!inserted && it->second != id) out.push_back({id, "serialized_key_collision"});
        }
    }
    return out;
}

std::string Manifest::glossary() const {
    std::ostringstream out;
    out << "terminology_version=" << version_ << '\n';
    for (const auto& [id, term] : terms_) {
        out << id << '|' << term.preferred << '|' << term.definition << '|' << term.owner << '|' << term.symbol << '|'
            << term.serializedKey << '\n';
    }
    return out.str();
}

Manifest Manifest::seeded() {
    Manifest manifest;
    auto add = [&](TermId id, const char* name, const char* definition, const char* owner, const char* key) {
        manifest.add({id, name, definition, owner, "", key, {}, {}, false});
    };
    add(1, "Macrovoxel", "Authoritative one-metre terrain/build cell.", "AG-VOX-001", "macrovoxel");
    add(2, "Microvoxel", "One optional refined subcell inside a macrovoxel.", "AG-VOX-002", "microvoxel");
    add(3, "Chunk", "Sparse streaming/storage unit of macrovoxels.", "AG-WORLD-001", "chunk");
    add(4, "Detail cluster", "Regenerable near-field presentation density, non-authoritative until promoted.", "AG-DETAIL-001", "detail_cluster");
    add(5, "Cube-sphere", "Planet address/projection model using direction-space ownership.", "AG-PLANET-001", "cube_sphere");
    add(6, "Field LOD", "Distant terrain representation queried from procedural fields.", "AG-LOD-001", "field_lod");
    add(7, "Claim", "Registry-Beacon ownership state; does not gate persistence.", "AG-STANDING-001", "claim");
    add(8, "Suspicion", "Per-system Imperial attention value.", "AG-STANDING-001", "suspicion");
    add(9, "Favor", "Galactic Unsworn/Court-facing reputation value.", "AG-STANDING-001", "favor");
    add(10, "Register Action", "Announced Imperial enforcement against a claim.", "AG-REGISTER-001", "register_action");
    add(11, "Touched chunk", "Chunk with durable player-authored deltas.", "AG-PERSIST-001", "touched_chunk");
    add(12, "Tombstone", "Durable record that generated identity was removed.", "AG-PERSIST-002", "tombstone");
    add(13, "Stable ID", "Save/network identity independent of transient ECS entity values.", "AG-ARCH-001", "stable_id");
    add(14, "Generator fingerprint", "Compatibility signature of procedural baseline behavior.", "AG-PERSIST-004", "generator_fingerprint");
    return manifest;
}

} // namespace elysium::terms
