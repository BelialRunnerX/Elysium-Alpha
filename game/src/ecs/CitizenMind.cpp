// Intended function: imported ecs implementation for CitizenMind; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/CitizenMind.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

constexpr SimTimeMs kThoughtLifetimeMs = 10 * 60 * 1000;
constexpr SimTimeMs kDayMs = 24LL * 60LL * 60LL * 1000LL;
constexpr float kMinFocus = 0.15f;
constexpr float kStrained = 25.0f;
constexpr float kDistressed = 50.0f;
constexpr float kCrisis = 75.0f;
constexpr float kRecoveryComplete = 15.0f;

bool finite(float value) { return std::isfinite(value); }
float clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }
float clampSigned(float value) { return std::clamp(value, -1.0f, 1.0f); }

std::size_t idx(NeedType value) { return static_cast<std::size_t>(value); }
std::size_t idx(PersonalityFacet value) { return static_cast<std::size_t>(value); }
std::size_t idx(ValueDimension value) { return static_cast<std::size_t>(value); }
std::size_t idx(CrisisResponse value) { return static_cast<std::size_t>(value); }

void setError(std::string* error, std::string_view message) {
    if (error) *error = std::string(message);
}

bool preferenceLess(const PreferenceEntry& a, const PreferenceEntry& b) {
    return std::tie(a.category, a.key) < std::tie(b.category, b.key);
}

bool thoughtLess(const ThoughtRecord& a, const ThoughtRecord& b) {
    return std::tie(a.createdAt, a.sourceEventId, a.topic) < std::tie(b.createdAt, b.sourceEventId, b.topic);
}

bool memoryLess(const MemoryRecord& a, const MemoryRecord& b) {
    return std::tie(a.formedAt, a.sourceEventId, a.topic) < std::tie(b.formedAt, b.sourceEventId, b.topic);
}

StressBand bandFor(float stress, StressBand oldBand) {
    if (stress >= kCrisis) return StressBand::Crisis;
    if ((oldBand == StressBand::Crisis || oldBand == StressBand::Distressed || oldBand == StressBand::Recovery)
        && stress < kDistressed && stress >= kRecoveryComplete) {
        return StressBand::Recovery;
    }
    if (stress >= kDistressed) return StressBand::Distressed;
    if (stress >= kStrained) return StressBand::Strained;
    return StressBand::Stable;
}

void appendBandTransition(CitizenMindState& state,
                          MindUpdateResult& result,
                          SimTimeMs now,
                          ChronicleEventId sourceEventId) {
    const StressBand desired = bandFor(state.stress.accumulated, state.stress.band);
    if (desired == state.stress.band) return;

    state.stress.previousBand = state.stress.band;
    state.stress.band = desired;
    state.stress.bandChangedAt = now;
    result.stressBandChanged = true;
    result.events.push_back({MindDomainEventType::StressBandChanged,
                             state.stableId,
                             sourceEventId,
                             now,
                             state.stress.accumulated,
                             std::nullopt,
                             desired,
                             CrisisResponse::Panic});

    if (desired == StressBand::Crisis) {
        const CrisisResponse response = CitizenMindSystem::selectCrisisResponse(state);
        state.stress.lastCrisisResponse = response;
        result.crisisIntentCreated = true;
        result.crisisResponse = response;
        result.events.push_back({MindDomainEventType::CrisisIntentCreated,
                                 state.stableId,
                                 sourceEventId,
                                 now,
                                 state.stress.accumulated,
                                 std::nullopt,
                                 desired,
                                 response});
    }
}

void appendThought(CurrentThoughts& thoughts, ThoughtRecord thought) {
    thoughts.entries.push_back(std::move(thought));
    if (thoughts.entries.size() <= CitizenMindSystem::kMaxCurrentThoughts) return;

    // Keep the strongest recent observations. Tie-breakers are explicit.
    auto weakest = std::min_element(thoughts.entries.begin(), thoughts.entries.end(), [](const auto& a, const auto& b) {
        if (a.intensity != b.intensity) return a.intensity < b.intensity;
        if (a.createdAt != b.createdAt) return a.createdAt < b.createdAt;
        return a.sourceEventId < b.sourceEventId;
    });
    thoughts.entries.erase(weakest);
}

void appendMemory(Memories& memories, MemoryRecord memory) {
    auto duplicate = std::find_if(memories.entries.begin(), memories.entries.end(), [&](const MemoryRecord& existing) {
        return memory.sourceEventId != 0 && existing.sourceEventId == memory.sourceEventId;
    });
    if (duplicate != memories.entries.end()) {
        if (memory.significance > duplicate->significance) *duplicate = std::move(memory);
        return;
    }

    memories.entries.push_back(std::move(memory));
    if (memories.entries.size() <= CitizenMindSystem::kMaxMemories) return;

    auto weakest = std::min_element(memories.entries.begin(), memories.entries.end(), [](const auto& a, const auto& b) {
        if (a.traumatic != b.traumatic) return !a.traumatic && b.traumatic;
        if (a.significance != b.significance) return a.significance < b.significance;
        if (a.formedAt != b.formedAt) return a.formedAt < b.formedAt;
        return a.sourceEventId < b.sourceEventId;
    });
    memories.entries.erase(weakest);
}

float needBurden(const CitizenMindState& state) {
    float weighted = 0.0f;
    float total = 0.0f;
    for (const auto& need : state.needs.dimensions) {
        weighted += clamp01(need.strength) * (1.0f - clamp01(need.satisfaction));
        total += clamp01(need.strength);
    }
    return total > 0.0001f ? weighted / total : 0.0f;
}

ThoughtKind defaultThoughtKind(MindEventType type, float valence) {
    switch (type) {
        case MindEventType::Danger: return ThoughtKind::Fear;
        case MindEventType::Injury: return ThoughtKind::Fear;
        case MindEventType::Loss: return ThoughtKind::Grief;
        case MindEventType::Achievement: return ThoughtKind::Pride;
        case MindEventType::Comfort: return ThoughtKind::Comfort;
        case MindEventType::SocialInteraction: return ThoughtKind::Social;
        case MindEventType::Conflict: return ThoughtKind::Conflict;
        case MindEventType::CreativeAct: return ThoughtKind::Inspiration;
        case MindEventType::Learning: return ThoughtKind::Interest;
        case MindEventType::Philosophy: return ThoughtKind::Comfort;
        case MindEventType::Discovery: return ThoughtKind::Interest;
        case MindEventType::WorkSuccess: return ThoughtKind::Pride;
        case MindEventType::WorkFailure: return ThoughtKind::Frustration;
        case MindEventType::NeedThreshold: return valence < 0.0f ? ThoughtKind::NeedDiscomfort : ThoughtKind::NeedRelief;
        case MindEventType::RelationshipChange: return valence < 0.0f ? ThoughtKind::Conflict : ThoughtKind::Social;
        case MindEventType::General: break;
    }
    return ThoughtKind::General;
}

float reactionScale(const CitizenMindState& state, const MindEvent& event) {
    float scale = 1.0f;
    if (event.reaction.facet) {
        scale += 0.50f * clamp01(state.personality.facets[idx(*event.reaction.facet)])
                       * clampSigned(event.reaction.facetWeight);
    }
    if (event.reaction.value) {
        scale += 0.35f * clamp01(state.values.importance[idx(*event.reaction.value)])
                       * clampSigned(event.reaction.valueWeight);
    }
    if (event.reaction.preferenceCategory && !event.reaction.preferenceKey.empty()) {
        const float preference = CitizenMindSystem::preferenceAffinity(state.preferences,
                                                                       *event.reaction.preferenceCategory,
                                                                       event.reaction.preferenceKey);
        scale += 0.35f * preference * clampSigned(event.reaction.preferenceWeight);
    }
    if (event.valence < 0.0f) {
        scale += 0.30f * clamp01(state.personality.facets[idx(PersonalityFacet::Anxiety)]);
    } else if (event.valence > 0.0f) {
        scale += 0.12f * clamp01(state.personality.facets[idx(PersonalityFacet::Curiosity)]);
    }
    return std::clamp(scale, 0.25f, 2.25f);
}

float memorySignificance(const MindEvent& event, float reactedIntensity) {
    float value = reactedIntensity * (0.65f + 0.35f * clamp01(event.chronicleImportance));
    if (event.type == MindEventType::Loss || event.type == MindEventType::Achievement
        || event.type == MindEventType::RelationshipChange) {
        value += 0.20f;
    }
    return clamp01(value);
}

InterventionKind interventionForNeed(NeedType need) {
    switch (need) {
        case NeedType::Sleep: return InterventionKind::Sleep;
        case NeedType::Food: return InterventionKind::Food;
        case NeedType::Safety: return InterventionKind::Safety;
        case NeedType::SocialContact: return InterventionKind::SocialContact;
        case NeedType::Purpose: return InterventionKind::Purpose;
        case NeedType::Philosophy: return InterventionKind::Philosophy;
        case NeedType::Creativity: return InterventionKind::Creativity;
        case NeedType::Learning: return InterventionKind::Learning;
        case NeedType::Family: return InterventionKind::Family;
        case NeedType::Solitude: return InterventionKind::Solitude;
        case NeedType::Excitement: return InterventionKind::Excitement;
        case NeedType::Count: break;
    }
    return InterventionKind::QuietRecovery;
}

bool relationshipKeyLess(const RelationshipEdge& a, const RelationshipEdge& b) {
    return std::tie(a.source, a.target) < std::tie(b.source, b.target);
}

bool hasType(std::uint32_t tags, RelationshipType type) {
    return (tags & relationshipMask(type)) != 0;
}

bool supportsRelationshipDecay(std::uint32_t tags) {
    constexpr std::uint32_t decaying = relationshipMask(RelationshipType::Friend)
        | relationshipMask(RelationshipType::Rival)
        | relationshipMask(RelationshipType::Coworker)
        | relationshipMask(RelationshipType::Creditor);
    return (tags & decaying) != 0;
}

float moveTowardZero(float value, float amount) {
    if (value > 0.0f) return std::max(0.0f, value - amount);
    if (value < 0.0f) return std::min(0.0f, value + amount);
    return 0.0f;
}

class ByteWriter {
public:
    std::vector<std::uint8_t> bytes;

    void raw(std::string_view text) { bytes.insert(bytes.end(), text.begin(), text.end()); }
    void u8(std::uint8_t value) { bytes.push_back(value); }
    void u32(std::uint32_t value) {
        for (int i = 0; i < 4; ++i) bytes.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
    }
    void u64(std::uint64_t value) {
        for (int i = 0; i < 8; ++i) bytes.push_back(static_cast<std::uint8_t>((value >> (i * 8)) & 0xFFu));
    }
    void i64(std::int64_t value) { u64(std::bit_cast<std::uint64_t>(value)); }
    void f32(float value) { u32(std::bit_cast<std::uint32_t>(value)); }
    bool string(std::string_view value, std::string* error) {
        if (value.size() > 65535u) {
            setError(error, "mind persistence string exceeds 65535 bytes");
            return false;
        }
        u32(static_cast<std::uint32_t>(value.size()));
        raw(value);
        return true;
    }
};

class ByteReader {
public:
    explicit ByteReader(std::span<const std::uint8_t> input) : input_(input) {}

    bool raw(std::string_view expected) {
        if (remaining() < expected.size()) return false;
        if (std::memcmp(input_.data() + pos_, expected.data(), expected.size()) != 0) return false;
        pos_ += expected.size();
        return true;
    }
    bool u8(std::uint8_t& value) {
        if (remaining() < 1) return false;
        value = input_[pos_++];
        return true;
    }
    bool u32(std::uint32_t& value) {
        if (remaining() < 4) return false;
        value = 0;
        for (int i = 0; i < 4; ++i) value |= static_cast<std::uint32_t>(input_[pos_++]) << (i * 8);
        return true;
    }
    bool u64(std::uint64_t& value) {
        if (remaining() < 8) return false;
        value = 0;
        for (int i = 0; i < 8; ++i) value |= static_cast<std::uint64_t>(input_[pos_++]) << (i * 8);
        return true;
    }
    bool i64(std::int64_t& value) {
        std::uint64_t bits{};
        if (!u64(bits)) return false;
        value = std::bit_cast<std::int64_t>(bits);
        return true;
    }
    bool f32(float& value) {
        std::uint32_t bits{};
        if (!u32(bits)) return false;
        value = std::bit_cast<float>(bits);
        return true;
    }
    bool string(std::string& value, std::size_t max = 65535u) {
        std::uint32_t length{};
        if (!u32(length) || length > max || remaining() < length) return false;
        value.assign(reinterpret_cast<const char*>(input_.data() + pos_), length);
        pos_ += length;
        return true;
    }
    bool done() const { return pos_ == input_.size(); }

private:
    std::size_t remaining() const { return input_.size() - pos_; }
    std::span<const std::uint8_t> input_;
    std::size_t pos_{};
};

void writeOptionalNeed(ByteWriter& writer, const std::optional<NeedType>& need) {
    writer.u8(need ? 1u : 0u);
    if (need) writer.u8(static_cast<std::uint8_t>(*need));
}

bool readOptionalNeed(ByteReader& reader, std::optional<NeedType>& need) {
    std::uint8_t present{};
    if (!reader.u8(present) || present > 1u) return false;
    if (!present) {
        need.reset();
        return true;
    }
    std::uint8_t raw{};
    if (!reader.u8(raw) || raw >= static_cast<std::uint8_t>(NeedType::Count)) return false;
    need = static_cast<NeedType>(raw);
    return true;
}

bool writeThought(ByteWriter& writer, const ThoughtRecord& thought, std::string* error) {
    writer.u64(thought.sourceEventId);
    writer.i64(thought.createdAt);
    writer.u8(static_cast<std::uint8_t>(thought.kind));
    writer.f32(thought.valence);
    writer.f32(thought.intensity);
    writeOptionalNeed(writer, thought.need);
    writer.u64(thought.other);
    return writer.string(thought.topic, error);
}

bool readThought(ByteReader& reader, ThoughtRecord& thought) {
    std::uint8_t kind{};
    if (!reader.u64(thought.sourceEventId) || !reader.i64(thought.createdAt) || !reader.u8(kind)
        || kind > static_cast<std::uint8_t>(ThoughtKind::General)
        || !reader.f32(thought.valence) || !reader.f32(thought.intensity)
        || !readOptionalNeed(reader, thought.need) || !reader.u64(thought.other)
        || !reader.string(thought.topic)) {
        return false;
    }
    thought.kind = static_cast<ThoughtKind>(kind);
    return finite(thought.valence) && finite(thought.intensity)
        && thought.valence >= -1.0f && thought.valence <= 1.0f
        && thought.intensity >= 0.0f && thought.intensity <= 1.0f;
}

bool writeMemory(ByteWriter& writer, const MemoryRecord& memory, std::string* error) {
    writer.u64(memory.sourceEventId);
    writer.i64(memory.formedAt);
    writer.u8(static_cast<std::uint8_t>(memory.kind));
    writer.f32(memory.valence);
    writer.f32(memory.strength);
    writer.f32(memory.significance);
    writer.u8(memory.traumatic ? 1u : 0u);
    writeOptionalNeed(writer, memory.need);
    writer.u64(memory.other);
    return writer.string(memory.topic, error);
}

bool readMemory(ByteReader& reader, MemoryRecord& memory) {
    std::uint8_t kind{};
    std::uint8_t traumatic{};
    if (!reader.u64(memory.sourceEventId) || !reader.i64(memory.formedAt) || !reader.u8(kind)
        || kind > static_cast<std::uint8_t>(ThoughtKind::General)
        || !reader.f32(memory.valence) || !reader.f32(memory.strength)
        || !reader.f32(memory.significance) || !reader.u8(traumatic) || traumatic > 1u
        || !readOptionalNeed(reader, memory.need) || !reader.u64(memory.other)
        || !reader.string(memory.topic)) {
        return false;
    }
    memory.kind = static_cast<ThoughtKind>(kind);
    memory.traumatic = traumatic != 0;
    return finite(memory.valence) && finite(memory.strength) && finite(memory.significance)
        && memory.valence >= -1.0f && memory.valence <= 1.0f
        && memory.strength >= 0.0f && memory.strength <= 1.0f
        && memory.significance >= 0.0f && memory.significance <= 1.0f;
}

bool validateMindState(const CitizenMindState& state, std::string* error) {
    if (state.stableId == 0) {
        setError(error, "citizen mind stable ID must be non-zero");
        return false;
    }
    for (float v : state.personality.facets) {
        if (!finite(v) || v < 0.0f || v > 1.0f) {
            setError(error, "personality facet outside [0,1]");
            return false;
        }
    }
    for (float v : state.values.importance) {
        if (!finite(v) || v < 0.0f || v > 1.0f) {
            setError(error, "value importance outside [0,1]");
            return false;
        }
    }
    if (state.preferences.entries.size() > 256u) {
        setError(error, "too many citizen preferences");
        return false;
    }
    for (const auto& pref : state.preferences.entries) {
        if (pref.key.empty() || !finite(pref.affinity) || pref.affinity < -1.0f || pref.affinity > 1.0f) {
            setError(error, "invalid preference entry");
            return false;
        }
    }
    for (const auto& need : state.needs.dimensions) {
        if (!finite(need.strength) || !finite(need.satisfaction)
            || need.strength < 0.0f || need.strength > 1.0f
            || need.satisfaction < 0.0f || need.satisfaction > 1.0f) {
            setError(error, "invalid need state");
            return false;
        }
    }
    if (!finite(state.stress.accumulated) || state.stress.accumulated < 0.0f || state.stress.accumulated > 100.0f) {
        setError(error, "invalid stress value");
        return false;
    }
    if (state.thoughts.entries.size() > CitizenMindSystem::kMaxCurrentThoughts
        || state.memories.entries.size() > CitizenMindSystem::kMaxMemories) {
        setError(error, "mind side-store exceeds bounded record capacity");
        return false;
    }
    for (float v : state.coping.responseAffinity) {
        if (!finite(v) || v < -1.0f || v > 1.0f) {
            setError(error, "invalid coping affinity");
            return false;
        }
    }
    return true;
}

} // namespace

const char* needTypeName(NeedType value) {
    switch (value) {
        case NeedType::Sleep: return "sleep";
        case NeedType::Food: return "food";
        case NeedType::Safety: return "safety";
        case NeedType::SocialContact: return "social contact";
        case NeedType::Purpose: return "purpose";
        case NeedType::Philosophy: return "philosophy/worship";
        case NeedType::Creativity: return "creativity";
        case NeedType::Learning: return "learning";
        case NeedType::Family: return "family";
        case NeedType::Solitude: return "solitude";
        case NeedType::Excitement: return "excitement";
        case NeedType::Count: break;
    }
    return "unknown need";
}

const char* stressBandName(StressBand value) {
    switch (value) {
        case StressBand::Stable: return "Stable";
        case StressBand::Strained: return "Strained";
        case StressBand::Distressed: return "Distressed";
        case StressBand::Crisis: return "Crisis";
        case StressBand::Recovery: return "Recovery";
    }
    return "Unknown";
}

const char* crisisResponseName(CrisisResponse value) {
    switch (value) {
        case CrisisResponse::Panic: return "panic";
        case CrisisResponse::Rage: return "rage";
        case CrisisResponse::Shutdown: return "shutdown";
        case CrisisResponse::Flight: return "flight";
        case CrisisResponse::Obsession: return "obsession";
    }
    return "unknown";
}

const char* mindWorkBlockerName(MindWorkBlocker value) {
    switch (value) {
        case MindWorkBlocker::None: return "none";
        case MindWorkBlocker::Crisis: return "crisis";
        case MindWorkBlocker::Distressed: return "distress/low focus";
        case MindWorkBlocker::ExhaustedNeed: return "exhausted sleep/food need";
        case MindWorkBlocker::UnsafeNeed: return "unsafe environment need";
        case MindWorkBlocker::LowFocus: return "low focus";
    }
    return "unknown";
}

const char* interventionName(InterventionKind value) {
    switch (value) {
        case InterventionKind::Sleep: return "provide sleep/rest";
        case InterventionKind::Food: return "provide food";
        case InterventionKind::Safety: return "restore safety/shelter";
        case InterventionKind::SocialContact: return "provide social contact";
        case InterventionKind::Purpose: return "provide purposeful activity";
        case InterventionKind::Philosophy: return "provide philosophy/worship access";
        case InterventionKind::Creativity: return "provide creative activity";
        case InterventionKind::Learning: return "provide learning access";
        case InterventionKind::Family: return "restore family contact";
        case InterventionKind::Solitude: return "provide solitude";
        case InterventionKind::Excitement: return "provide stimulation/excitement";
        case InterventionKind::CounselingOrCoping: return "provide coping/social support";
        case InterventionKind::ResolveGrievance: return "resolve grievance/conflict";
        case InterventionKind::MemorialOrGriefSupport: return "provide grief/memorial support";
        case InterventionKind::MedicalSafety: return "address injury/medical safety";
        case InterventionKind::QuietRecovery: return "allow quiet recovery time";
    }
    return "unknown intervention";
}

void CitizenMindSystem::normalize(CitizenMindState& state) {
    for (float& v : state.personality.facets) v = finite(v) ? clamp01(v) : 0.5f;
    for (float& v : state.values.importance) v = finite(v) ? clamp01(v) : 0.5f;
    for (auto& need : state.needs.dimensions) {
        need.strength = finite(need.strength) ? clamp01(need.strength) : 0.5f;
        need.satisfaction = finite(need.satisfaction) ? clamp01(need.satisfaction) : 1.0f;
    }
    for (auto& pref : state.preferences.entries) pref.affinity = finite(pref.affinity) ? clampSigned(pref.affinity) : 0.0f;
    std::sort(state.preferences.entries.begin(), state.preferences.entries.end(), preferenceLess);
    state.preferences.entries.erase(std::unique(state.preferences.entries.begin(), state.preferences.entries.end(), [](const auto& a, const auto& b) {
        return a.category == b.category && a.key == b.key;
    }), state.preferences.entries.end());
    for (float& v : state.coping.responseAffinity) v = finite(v) ? clampSigned(v) : 0.0f;
    state.stress.accumulated = finite(state.stress.accumulated) ? std::clamp(state.stress.accumulated, 0.0f, 100.0f) : 0.0f;
    std::sort(state.thoughts.entries.begin(), state.thoughts.entries.end(), thoughtLess);
    if (state.thoughts.entries.size() > kMaxCurrentThoughts) {
        state.thoughts.entries.erase(state.thoughts.entries.begin(), state.thoughts.entries.end() - static_cast<std::ptrdiff_t>(kMaxCurrentThoughts));
    }
    std::sort(state.memories.entries.begin(), state.memories.entries.end(), memoryLess);
    if (state.memories.entries.size() > kMaxMemories) {
        state.memories.entries.erase(state.memories.entries.begin(), state.memories.entries.end() - static_cast<std::ptrdiff_t>(kMaxMemories));
    }
    recomputeFocus(state);
}

float CitizenMindSystem::preferenceAffinity(const Preferences& preferences,
                                            PreferenceCategory category,
                                            std::string_view key) {
    const auto it = std::lower_bound(preferences.entries.begin(), preferences.entries.end(),
                                     PreferenceEntry{category, std::string(key), 0.0f}, preferenceLess);
    if (it != preferences.entries.end() && it->category == category && it->key == key) return clampSigned(it->affinity);
    return 0.0f;
}

bool CitizenMindSystem::setPreference(Preferences& preferences, PreferenceEntry entry, std::string* error) {
    if (entry.key.empty()) {
        setError(error, "preference key must not be empty");
        return false;
    }
    if (!finite(entry.affinity)) {
        setError(error, "preference affinity must be finite");
        return false;
    }
    entry.affinity = clampSigned(entry.affinity);
    auto it = std::lower_bound(preferences.entries.begin(), preferences.entries.end(), entry, preferenceLess);
    if (it != preferences.entries.end() && it->category == entry.category && it->key == entry.key) {
        it->affinity = entry.affinity;
    } else {
        preferences.entries.insert(it, std::move(entry));
    }
    return true;
}

MindUpdateResult CitizenMindSystem::updateNeed(CitizenMindState& state,
                                               NeedType need,
                                               float newSatisfaction,
                                               SimTimeMs now,
                                               ChronicleEventId eventId) {
    MindUpdateResult result{};
    if (need == NeedType::Count || !finite(newSatisfaction)) return result;
    auto& dimension = state.needs.dimensions[idx(need)];
    const float old = clamp01(dimension.satisfaction);
    dimension.satisfaction = clamp01(newSatisfaction);

    const bool becameProblem = old >= kNeedProblemThreshold && dimension.satisfaction < kNeedProblemThreshold;
    const bool becameRelieved = old <= kNeedReliefThreshold && dimension.satisfaction > kNeedReliefThreshold;
    if (!becameProblem && !becameRelieved) {
        recomputeFocus(state);
        return result;
    }

    MindEvent event{};
    event.eventId = eventId;
    event.time = now;
    event.type = MindEventType::NeedThreshold;
    event.need = need;
    event.valence = becameProblem ? -1.0f : 0.65f;
    event.intensity = becameProblem ? clamp01(1.0f - dimension.satisfaction)
                                    : clamp01(dimension.satisfaction);
    event.thoughtKind = becameProblem ? ThoughtKind::NeedDiscomfort : ThoughtKind::NeedRelief;
    event.topic = needTypeName(need);
    event.reaction.facet = need == NeedType::SocialContact ? std::optional{PersonalityFacet::Sociability}
                                                           : std::optional{PersonalityFacet::Patience};
    event.reaction.facetWeight = becameProblem ? 0.35f : 0.15f;

    result = applyEvent(state, event);
    result.events.insert(result.events.begin(), {MindDomainEventType::NeedThresholdCrossed,
                                                 state.stableId,
                                                 eventId,
                                                 now,
                                                 dimension.satisfaction,
                                                 need,
                                                 state.stress.band,
                                                 state.stress.lastCrisisResponse});
    recomputeFocus(state);
    return result;
}

MindUpdateResult CitizenMindSystem::applyEvent(CitizenMindState& state, const MindEvent& event) {
    MindUpdateResult result{};
    if (state.stableId == 0 || !finite(event.valence) || !finite(event.intensity)
        || !finite(event.chronicleImportance)) {
        return result;
    }

    const float valence = clampSigned(event.valence);
    const float scale = reactionScale(state, event);
    const float intensity = clamp01(clamp01(event.intensity) * scale);
    const ThoughtKind kind = (event.thoughtKind == ThoughtKind::General)
        ? defaultThoughtKind(event.type, valence) : event.thoughtKind;

    ThoughtRecord thought{};
    thought.sourceEventId = event.eventId;
    thought.createdAt = event.time;
    thought.kind = kind;
    thought.valence = valence;
    thought.intensity = intensity;
    thought.need = event.need;
    thought.other = event.other;
    thought.topic = event.topic;
    appendThought(state.thoughts, std::move(thought));
    result.thoughtCreated = true;
    result.events.push_back({MindDomainEventType::ThoughtCreated,
                             state.stableId,
                             event.eventId,
                             event.time,
                             intensity,
                             event.need,
                             state.stress.band,
                             state.stress.lastCrisisResponse});

    const float anxiety = clamp01(state.personality.facets[idx(PersonalityFacet::Anxiety)]);
    const float anger = clamp01(state.personality.facets[idx(PersonalityFacet::Anger)]);
    const float discipline = clamp01(state.personality.facets[idx(PersonalityFacet::Discipline)]);
    const float patience = clamp01(state.personality.facets[idx(PersonalityFacet::Patience)]);
    float delta = 0.0f;
    if (valence < 0.0f) {
        const float vulnerability = std::clamp(0.75f + 0.65f * anxiety + 0.25f * anger - 0.25f * discipline, 0.35f, 1.8f);
        delta = 22.0f * intensity * (-valence) * vulnerability;
    } else if (valence > 0.0f) {
        const float recovery = 0.60f + 0.40f * patience + 0.20f * discipline;
        delta = -8.0f * intensity * valence * recovery;
    }
    delta = std::clamp(delta, -20.0f, 40.0f);
    const float before = state.stress.accumulated;
    state.stress.accumulated = std::clamp(before + delta, 0.0f, 100.0f);
    result.stressDelta = state.stress.accumulated - before;

    const float significance = memorySignificance(event, intensity);
    if (significance >= kMemoryPromotionThreshold || event.type == MindEventType::Loss) {
        MemoryRecord memory{};
        memory.sourceEventId = event.eventId;
        memory.formedAt = event.time;
        memory.kind = kind;
        memory.valence = valence;
        memory.strength = significance;
        memory.significance = significance;
        memory.traumatic = valence < 0.0f && (significance >= 0.78f || event.type == MindEventType::Loss);
        memory.need = event.need;
        memory.other = event.other;
        memory.topic = event.topic;
        appendMemory(state.memories, std::move(memory));
        result.memoryFormed = true;
        result.events.push_back({MindDomainEventType::MemoryFormed,
                                 state.stableId,
                                 event.eventId,
                                 event.time,
                                 significance,
                                 event.need,
                                 state.stress.band,
                                 state.stress.lastCrisisResponse});
    }

    appendBandTransition(state, result, event.time, event.eventId);
    recomputeFocus(state);
    return result;
}

MindUpdateResult CitizenMindSystem::tick(CitizenMindState& state, SimTimeMs now, SimTimeMs elapsedMs) {
    MindUpdateResult result{};
    if (elapsedMs <= 0 || state.stableId == 0) return result;

    state.thoughts.entries.erase(std::remove_if(state.thoughts.entries.begin(), state.thoughts.entries.end(), [&](const ThoughtRecord& thought) {
        return now >= thought.createdAt && (now - thought.createdAt) > kThoughtLifetimeMs;
    }), state.thoughts.entries.end());

    const float seconds = static_cast<float>(elapsedMs) / 1000.0f;
    const float burden = needBurden(state);
    const float anxiety = clamp01(state.personality.facets[idx(PersonalityFacet::Anxiety)]);
    const float discipline = clamp01(state.personality.facets[idx(PersonalityFacet::Discipline)]);

    float negativeMemoryLoad = 0.0f;
    for (const auto& memory : state.memories.entries) {
        if (memory.valence >= 0.0f) continue;
        negativeMemoryLoad += effectiveMemoryStrength(memory, now) * (-memory.valence);
    }
    negativeMemoryLoad = std::min(4.0f, negativeMemoryLoad);

    const float accumulationPerSecond = std::max(0.0f, burden - 0.22f) * (0.095f + 0.055f * anxiety)
                                      + negativeMemoryLoad * 0.0025f;
    const float relaxationPerSecond = (1.0f - burden) * (0.028f + 0.020f * discipline);
    const float delta = (accumulationPerSecond - relaxationPerSecond) * seconds;
    const float before = state.stress.accumulated;
    state.stress.accumulated = std::clamp(before + delta, 0.0f, 100.0f);
    result.stressDelta = state.stress.accumulated - before;

    appendBandTransition(state, result, now, 0);
    recomputeFocus(state);
    return result;
}

float CitizenMindSystem::effectiveMemoryStrength(const MemoryRecord& memory, SimTimeMs now) {
    if (now <= memory.formedAt) return clamp01(memory.strength);
    const double ageDays = static_cast<double>(now - memory.formedAt) / static_cast<double>(kDayMs);
    const double halfLife = memory.traumatic ? 180.0 : 30.0;
    const float decayed = static_cast<float>(clamp01(memory.strength) * std::pow(0.5, ageDays / halfLife));
    if (memory.traumatic) return std::max(0.35f * clamp01(memory.strength), decayed);
    return decayed;
}

float CitizenMindSystem::recomputeFocus(CitizenMindState& state) {
    const float burden = needBurden(state);
    const float stress = std::clamp(state.stress.accumulated / 100.0f, 0.0f, 1.0f);
    state.focus.value = std::clamp(1.0f - 0.52f * burden - 0.43f * stress, kMinFocus, 1.0f);
    return state.focus.value;
}

CrisisResponse CitizenMindSystem::selectCrisisResponse(const CitizenMindState& state) {
    const auto facet = [&](PersonalityFacet f) { return clamp01(state.personality.facets[idx(f)]); };
    std::array<float, 5> scores{};
    scores[idx(CrisisResponse::Panic)] = 0.75f * facet(PersonalityFacet::Anxiety) + 0.20f * (1.0f - facet(PersonalityFacet::Discipline));
    scores[idx(CrisisResponse::Rage)] = 0.85f * facet(PersonalityFacet::Anger) + 0.20f * facet(PersonalityFacet::RiskTolerance);
    scores[idx(CrisisResponse::Shutdown)] = 0.45f * (1.0f - facet(PersonalityFacet::Sociability))
                                         + 0.35f * (1.0f - facet(PersonalityFacet::RiskTolerance))
                                         + 0.20f * (1.0f - facet(PersonalityFacet::Curiosity));
    scores[idx(CrisisResponse::Flight)] = 0.65f * facet(PersonalityFacet::Anxiety)
                                       + 0.45f * (1.0f - facet(PersonalityFacet::RiskTolerance));
    scores[idx(CrisisResponse::Obsession)] = 0.55f * facet(PersonalityFacet::Curiosity)
                                           + 0.35f * facet(PersonalityFacet::Discipline)
                                           + 0.25f * facet(PersonalityFacet::Orderliness);
    for (std::size_t i = 0; i < scores.size(); ++i) scores[i] += 0.35f * clampSigned(state.coping.responseAffinity[i]);

    std::size_t best = 0;
    for (std::size_t i = 1; i < scores.size(); ++i) {
        if (scores[i] > scores[best]) best = i; // enum order is deterministic tie-break.
    }
    return static_cast<CrisisResponse>(best);
}

WorkReadiness CitizenMindSystem::evaluateWorkReadiness(const CitizenMindState& state) {
    WorkReadiness result{};
    if (state.stress.band == StressBand::Crisis) {
        return {false, MindWorkBlocker::Crisis, std::nullopt, state.stress.accumulated / 100.0f};
    }
    if (state.stress.band == StressBand::Distressed && state.focus.value < 0.48f) {
        return {false, MindWorkBlocker::Distressed, std::nullopt, 1.0f - state.focus.value};
    }

    const auto& sleep = state.needs.dimensions[idx(NeedType::Sleep)];
    if (sleep.strength * (1.0f - sleep.satisfaction) > 0.72f) {
        return {false, MindWorkBlocker::ExhaustedNeed, NeedType::Sleep, 1.0f - sleep.satisfaction};
    }
    const auto& safety = state.needs.dimensions[idx(NeedType::Safety)];
    if (safety.strength * (1.0f - safety.satisfaction) > 0.75f) {
        return {false, MindWorkBlocker::UnsafeNeed, NeedType::Safety, 1.0f - safety.satisfaction};
    }
    if (state.focus.value < 0.30f) {
        return {false, MindWorkBlocker::LowFocus, std::nullopt, 1.0f - state.focus.value};
    }
    return result;
}

MindDiagnostic CitizenMindSystem::inspect(const CitizenMindState& state, SimTimeMs now, std::size_t maxReasons) {
    MindDiagnostic diagnostic{};
    diagnostic.stableId = state.stableId;
    diagnostic.band = state.stress.band;
    diagnostic.stress = state.stress.accumulated;
    diagnostic.focus = state.focus.value;
    diagnostic.work = evaluateWorkReadiness(state);

    for (std::size_t i = 0; i < state.needs.dimensions.size(); ++i) {
        const auto& need = state.needs.dimensions[i];
        const float weight = clamp01(need.strength) * (1.0f - clamp01(need.satisfaction));
        if (weight < 0.10f) continue;
        diagnostic.reasons.push_back({MindDiagnosticReason::Kind::UnmetNeed,
                                      weight,
                                      static_cast<NeedType>(i),
                                      0,
                                      needTypeName(static_cast<NeedType>(i))});
    }
    for (const auto& memory : state.memories.entries) {
        if (memory.valence >= 0.0f) continue;
        const float weight = effectiveMemoryStrength(memory, now) * (-memory.valence);
        if (weight < 0.08f) continue;
        diagnostic.reasons.push_back({MindDiagnosticReason::Kind::Memory,
                                      weight,
                                      memory.need,
                                      memory.sourceEventId,
                                      memory.topic});
    }
    for (const auto& thought : state.thoughts.entries) {
        if (thought.valence >= 0.0f) continue;
        const float weight = thought.intensity * (-thought.valence);
        if (weight < 0.08f) continue;
        diagnostic.reasons.push_back({MindDiagnosticReason::Kind::Thought,
                                      weight,
                                      thought.need,
                                      thought.sourceEventId,
                                      thought.topic});
    }
    diagnostic.reasons.push_back({MindDiagnosticReason::Kind::StressBand,
                                  state.stress.accumulated / 100.0f,
                                  std::nullopt,
                                  0,
                                  stressBandName(state.stress.band)});

    std::stable_sort(diagnostic.reasons.begin(), diagnostic.reasons.end(), [](const auto& a, const auto& b) {
        if (a.weight != b.weight) return a.weight > b.weight;
        if (a.kind != b.kind) return a.kind < b.kind;
        if (a.eventId != b.eventId) return a.eventId < b.eventId;
        return a.topic < b.topic;
    });
    if (diagnostic.reasons.size() > maxReasons) diagnostic.reasons.resize(maxReasons);

    for (const auto& reason : diagnostic.reasons) {
        if (reason.kind == MindDiagnosticReason::Kind::UnmetNeed && reason.need) {
            diagnostic.interventions.push_back(interventionForNeed(*reason.need));
        } else if (reason.kind == MindDiagnosticReason::Kind::Memory) {
            const auto it = std::find_if(state.memories.entries.begin(), state.memories.entries.end(), [&](const MemoryRecord& memory) {
                return memory.sourceEventId == reason.eventId;
            });
            if (it != state.memories.entries.end()) {
                if (it->kind == ThoughtKind::Grief) diagnostic.interventions.push_back(InterventionKind::MemorialOrGriefSupport);
                else if (it->kind == ThoughtKind::Conflict) diagnostic.interventions.push_back(InterventionKind::ResolveGrievance);
                else if (it->kind == ThoughtKind::Fear) diagnostic.interventions.push_back(InterventionKind::MedicalSafety);
            }
        }
    }
    if (state.stress.band == StressBand::Crisis || state.stress.band == StressBand::Distressed) {
        diagnostic.interventions.push_back(InterventionKind::CounselingOrCoping);
    } else if (state.stress.band == StressBand::Recovery) {
        diagnostic.interventions.push_back(InterventionKind::QuietRecovery);
    }
    std::sort(diagnostic.interventions.begin(), diagnostic.interventions.end());
    diagnostic.interventions.erase(std::unique(diagnostic.interventions.begin(), diagnostic.interventions.end()), diagnostic.interventions.end());
    return diagnostic;
}

std::vector<MindUpdateResult> CitizenMindSystem::tickBatch(JobSystem& jobs,
                                                           std::vector<CitizenMindState>& states,
                                                           SimTimeMs now,
                                                           SimTimeMs elapsedMs) {
    std::stable_sort(states.begin(), states.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    std::vector<MindUpdateResult> results(states.size());
    jobs.parallelFor(states.size(), [&](std::size_t i) {
        results[i] = tick(states[i], now, elapsedMs);
    }, 8);
    return results;
}

std::string formatMindDiagnostic(const MindDiagnostic& diagnostic) {
    std::ostringstream out;
    out << "citizen " << diagnostic.stableId
        << ": stress=" << std::fixed << std::setprecision(1) << diagnostic.stress
        << " (" << stressBandName(diagnostic.band) << ")"
        << ", focus=" << std::setprecision(2) << diagnostic.focus;
    if (!diagnostic.work.canWork) {
        out << ", work-blocked=" << mindWorkBlockerName(diagnostic.work.blocker);
        if (diagnostic.work.need) out << " (" << needTypeName(*diagnostic.work.need) << ")";
    }
    out << "\nreasons:";
    for (const auto& reason : diagnostic.reasons) {
        out << "\n - " << std::setprecision(2) << reason.weight << " ";
        if (reason.kind == MindDiagnosticReason::Kind::UnmetNeed && reason.need) out << "unmet " << needTypeName(*reason.need);
        else if (reason.kind == MindDiagnosticReason::Kind::Memory) out << "memory event " << reason.eventId << " [" << reason.topic << "]";
        else if (reason.kind == MindDiagnosticReason::Kind::Thought) out << "thought event " << reason.eventId << " [" << reason.topic << "]";
        else out << "stress band " << reason.topic;
    }
    out << "\nlikely interventions:";
    for (auto intervention : diagnostic.interventions) out << "\n - " << interventionName(intervention);
    return out.str();
}

// ---------------------------------------------------------------------------
// Relationship store
// ---------------------------------------------------------------------------

bool validateRelationshipEdge(const RelationshipEdge& edge, std::string* error) {
    if (edge.source == 0 || edge.target == 0 || edge.source == edge.target) {
        setError(error, "relationship endpoints must be distinct non-zero StableIds");
        return false;
    }
    if (edge.typeTags == 0) {
        setError(error, "relationship must contain at least one type tag");
        return false;
    }
    const auto inSigned = [](float v) { return finite(v) && v >= -1.0f && v <= 1.0f; };
    const auto inUnit = [](float v) { return finite(v) && v >= 0.0f && v <= 1.0f; };
    if (!inSigned(edge.affinity) || !inUnit(edge.trust) || !inUnit(edge.respect)
        || !inUnit(edge.fear) || !inUnit(edge.grievance) || !inUnit(edge.familiarity)) {
        setError(error, "relationship dimensions outside valid ranges");
        return false;
    }
    if (edge.eventRefs.size() > RelationshipStore::kMaxEventRefsPerEdge) {
        setError(error, "relationship event reference list exceeds bounded capacity");
        return false;
    }
    return true;
}

std::vector<RelationshipEdge>::iterator RelationshipStore::lowerBound(CitizenStableId source, CitizenStableId target) {
    return std::lower_bound(edges_.begin(), edges_.end(), std::pair{source, target}, [](const RelationshipEdge& edge, const auto& key) {
        return edge.source < key.first || (edge.source == key.first && edge.target < key.second);
    });
}

std::vector<RelationshipEdge>::const_iterator RelationshipStore::lowerBound(CitizenStableId source, CitizenStableId target) const {
    return std::lower_bound(edges_.begin(), edges_.end(), std::pair{source, target}, [](const RelationshipEdge& edge, const auto& key) {
        return edge.source < key.first || (edge.source == key.first && edge.target < key.second);
    });
}

bool RelationshipStore::upsert(RelationshipEdge edge, std::string* error) {
    if (!validateRelationshipEdge(edge, error)) return false;
    std::sort(edge.eventRefs.begin(), edge.eventRefs.end());
    edge.eventRefs.erase(std::unique(edge.eventRefs.begin(), edge.eventRefs.end()), edge.eventRefs.end());
    if (edge.eventRefs.size() > kMaxEventRefsPerEdge) {
        edge.eventRefs.erase(edge.eventRefs.begin(), edge.eventRefs.end() - static_cast<std::ptrdiff_t>(kMaxEventRefsPerEdge));
    }
    auto it = lowerBound(edge.source, edge.target);
    if (it != edges_.end() && it->source == edge.source && it->target == edge.target) *it = std::move(edge);
    else edges_.insert(it, std::move(edge));
    return true;
}

void RelationshipStore::emitLinkEvents(const RelationshipEdge& before, const RelationshipEdge& after,
                                       ChronicleEventId eventId, SimTimeMs now) {
    emittedEvents_.push_back({SocialDomainEventType::RelationshipChanged, after.source, after.target, eventId, now});
    const std::uint32_t added = after.typeTags & ~before.typeTags;
    if (added & relationshipMask(RelationshipType::Partner)) {
        emittedEvents_.push_back({SocialDomainEventType::PartnershipFormed, after.source, after.target, eventId, now});
    }
    if (added & relationshipMask(RelationshipType::Mentor)) {
        emittedEvents_.push_back({SocialDomainEventType::MentorshipEstablished, after.source, after.target, eventId, now});
    }
    if (before.grievance <= 0.0001f && after.grievance > 0.0001f) {
        emittedEvents_.push_back({SocialDomainEventType::GrievanceCreated, after.source, after.target, eventId, now});
    }
}

bool RelationshipStore::apply(CitizenStableId source,
                              CitizenStableId target,
                              const RelationshipDelta& delta,
                              SimTimeMs now,
                              ChronicleEventId eventId,
                              std::string* error) {
    if (source == 0 || target == 0 || source == target) {
        setError(error, "relationship mutation requires distinct non-zero StableIds");
        return false;
    }
    const auto boundedDelta = [](float v) { return finite(v) && v >= -0.5f && v <= 0.5f; };
    if (!boundedDelta(delta.affinity) || !boundedDelta(delta.trust) || !boundedDelta(delta.respect)
        || !boundedDelta(delta.fear) || !boundedDelta(delta.grievance) || !boundedDelta(delta.familiarity)) {
        setError(error, "relationship event delta exceeds +/-0.5 bound");
        return false;
    }

    RelationshipEdge before{};
    RelationshipEdge current{};
    if (auto effectiveEdge = effective(source, target, now)) {
        current = *effectiveEdge;
        before = current;
    } else {
        current.source = source;
        current.target = target;
        current.lastInteraction = now;
        before = current;
    }

    current.typeTags |= delta.addTypeTags;
    current.typeTags &= ~delta.removeTypeTags;
    if (current.typeTags == 0) {
        setError(error, "relationship mutation would remove every type tag");
        return false;
    }
    current.affinity = std::clamp(current.affinity + delta.affinity, -1.0f, 1.0f);
    current.trust = clamp01(current.trust + delta.trust);
    current.respect = clamp01(current.respect + delta.respect);
    current.fear = clamp01(current.fear + delta.fear);
    current.grievance = clamp01(current.grievance + delta.grievance);
    current.familiarity = clamp01(current.familiarity + delta.familiarity);
    current.lastInteraction = now;
    if (eventId != 0) {
        current.eventRefs.push_back(eventId);
        std::sort(current.eventRefs.begin(), current.eventRefs.end());
        current.eventRefs.erase(std::unique(current.eventRefs.begin(), current.eventRefs.end()), current.eventRefs.end());
        if (current.eventRefs.size() > kMaxEventRefsPerEdge) {
            current.eventRefs.erase(current.eventRefs.begin(), current.eventRefs.end() - static_cast<std::ptrdiff_t>(kMaxEventRefsPerEdge));
        }
    }
    if (!upsert(current, error)) return false;
    emitLinkEvents(before, current, eventId, now);
    return true;
}

bool RelationshipStore::linkParentChild(CitizenStableId parent, CitizenStableId child,
                                        SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta p{};
    p.addTypeTags = relationshipMask(RelationshipType::Parent);
    p.affinity = 0.15f; p.trust = 0.10f; p.familiarity = 0.25f;
    RelationshipDelta c = p;
    c.addTypeTags = relationshipMask(RelationshipType::Child);
    return apply(parent, child, p, now, eventId, error) && apply(child, parent, c, now, eventId, error);
}

bool RelationshipStore::linkSiblings(CitizenStableId a, CitizenStableId b,
                                     SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta d{};
    d.addTypeTags = relationshipMask(RelationshipType::Sibling);
    d.affinity = 0.10f; d.trust = 0.08f; d.familiarity = 0.20f;
    return apply(a, b, d, now, eventId, error) && apply(b, a, d, now, eventId, error);
}

bool RelationshipStore::linkGuardianWard(CitizenStableId guardian, CitizenStableId ward,
                                         SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta a{};
    a.addTypeTags = relationshipMask(RelationshipType::Guardian);
    a.affinity = 0.10f; a.trust = 0.10f; a.familiarity = 0.20f;
    RelationshipDelta b = a;
    b.addTypeTags = relationshipMask(RelationshipType::Ward);
    return apply(guardian, ward, a, now, eventId, error) && apply(ward, guardian, b, now, eventId, error);
}

bool RelationshipStore::linkPartner(CitizenStableId a, CitizenStableId b,
                                    SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta d{};
    d.addTypeTags = relationshipMask(RelationshipType::Partner);
    d.affinity = 0.20f; d.trust = 0.20f; d.familiarity = 0.25f;
    return apply(a, b, d, now, eventId, error) && apply(b, a, d, now, eventId, error);
}

bool RelationshipStore::linkFriend(CitizenStableId a, CitizenStableId b,
                                   SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta d{};
    d.addTypeTags = relationshipMask(RelationshipType::Friend);
    d.affinity = 0.12f; d.trust = 0.08f; d.familiarity = 0.20f;
    return apply(a, b, d, now, eventId, error) && apply(b, a, d, now, eventId, error);
}

bool RelationshipStore::linkCoworkers(CitizenStableId a, CitizenStableId b,
                                      SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta d{};
    d.addTypeTags = relationshipMask(RelationshipType::Coworker);
    d.trust = 0.04f; d.respect = 0.04f; d.familiarity = 0.10f;
    return apply(a, b, d, now, eventId, error) && apply(b, a, d, now, eventId, error);
}

bool RelationshipStore::linkRivals(CitizenStableId a, CitizenStableId b,
                                   SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta d{};
    d.addTypeTags = relationshipMask(RelationshipType::Rival);
    d.affinity = -0.15f; d.grievance = 0.15f; d.familiarity = 0.15f;
    return apply(a, b, d, now, eventId, error) && apply(b, a, d, now, eventId, error);
}

bool RelationshipStore::linkMentor(CitizenStableId mentor, CitizenStableId student,
                                   SimTimeMs now, ChronicleEventId eventId, std::string* error) {
    RelationshipDelta a{};
    a.addTypeTags = relationshipMask(RelationshipType::Mentor);
    a.respect = 0.10f; a.familiarity = 0.15f;
    // Mentor is directional. The student still sees this edge through
    // edgesFor(target), so inventing a reverse "mentee" or coworker tag is
    // unnecessary and would misstate the documented relationship vocabulary.
    return apply(mentor, student, a, now, eventId, error);
}

bool RelationshipStore::linkFamilyGeneration(CitizenStableId child,
                                             std::span<const CitizenStableId> parents,
                                             std::span<const CitizenStableId> siblings,
                                             std::span<const CitizenStableId> guardians,
                                             SimTimeMs now, ChronicleEventId eventId,
                                             std::string* error) {
    if (child == 0) {
        setError(error, "family generation requires a non-zero child StableId");
        return false;
    }

    // Canonicalize supplied identities so caller/container iteration order cannot
    // alter edge/event order or persistence bytes. This is intentionally generic:
    // species definitions, not Agent 18, decide family cardinality.
    const auto canonical = [child](std::span<const CitizenStableId> ids, std::string* err) {
        std::vector<CitizenStableId> out(ids.begin(), ids.end());
        std::sort(out.begin(), out.end());
        out.erase(std::unique(out.begin(), out.end()), out.end());
        if (std::find(out.begin(), out.end(), CitizenStableId{0}) != out.end()) {
            setError(err, "family generation contains zero StableId");
            out.clear();
            return out;
        }
        if (std::find(out.begin(), out.end(), child) != out.end()) {
            setError(err, "family generation cannot relate citizen to itself");
            out.clear();
            return out;
        }
        return out;
    };

    auto parentIds = canonical(parents, error);
    if (!parents.empty() && parentIds.empty()) return false;
    auto siblingIds = canonical(siblings, error);
    if (!siblings.empty() && siblingIds.empty()) return false;
    auto guardianIds = canonical(guardians, error);
    if (!guardians.empty() && guardianIds.empty()) return false;

    // Prevent ambiguous duplication across role sets. A person can be both parent
    // and guardian in fiction, but the Parent/Child edge already carries the durable
    // family role; adding Guardian/Ward in the same operation would create redundant
    // event churn and inconsistent callers. Existing edges may still carry both tags
    // when explicitly authored later.
    for (CitizenStableId id : parentIds) {
        guardianIds.erase(std::remove(guardianIds.begin(), guardianIds.end(), id), guardianIds.end());
        siblingIds.erase(std::remove(siblingIds.begin(), siblingIds.end(), id), siblingIds.end());
    }
    for (CitizenStableId id : guardianIds) {
        siblingIds.erase(std::remove(siblingIds.begin(), siblingIds.end(), id), siblingIds.end());
    }

    // Transaction-like preflight: all identities have been validated before any
    // edge is changed. The primitive link operations cannot fail after this point
    // under their bounded built-in deltas, so partial family creation is avoided.
    for (CitizenStableId parent : parentIds) {
        if (!linkParentChild(parent, child, now, eventId, error)) return false;
    }
    for (CitizenStableId guardian : guardianIds) {
        if (!linkGuardianWard(guardian, child, now, eventId, error)) return false;
    }
    for (CitizenStableId sibling : siblingIds) {
        if (!linkSiblings(child, sibling, now, eventId, error)) return false;
    }
    return true;
}

const RelationshipEdge* RelationshipStore::find(CitizenStableId source, CitizenStableId target) const {
    const auto it = lowerBound(source, target);
    if (it == edges_.end() || it->source != source || it->target != target) return nullptr;
    return &*it;
}

std::optional<RelationshipEdge> RelationshipStore::effective(CitizenStableId source,
                                                              CitizenStableId target,
                                                              SimTimeMs now) const {
    const auto* stored = find(source, target);
    if (!stored) return std::nullopt;
    RelationshipEdge edge = *stored;
    if (!supportsRelationshipDecay(edge.typeTags) || now <= edge.lastInteraction) return edge;

    const double days = static_cast<double>(now - edge.lastInteraction) / static_cast<double>(kDayMs);
    edge.familiarity = std::max(0.0f, edge.familiarity - static_cast<float>(days * 0.0020));
    edge.affinity = moveTowardZero(edge.affinity, static_cast<float>(days * 0.0010));
    edge.trust = std::max(0.0f, edge.trust - static_cast<float>(days * 0.0005));
    if (hasType(edge.typeTags, RelationshipType::Rival) || hasType(edge.typeTags, RelationshipType::Creditor)) {
        edge.grievance = std::max(0.0f, edge.grievance - static_cast<float>(days * 0.0010));
    }
    return edge;
}

std::vector<RelationshipEdge> RelationshipStore::edgesFor(CitizenStableId stableId, SimTimeMs now) const {
    std::vector<RelationshipEdge> result;
    for (const auto& edge : edges_) {
        if (edge.source != stableId && edge.target != stableId) continue;
        if (auto current = effective(edge.source, edge.target, now)) result.push_back(*current);
    }
    std::sort(result.begin(), result.end(), relationshipKeyLess);
    return result;
}

std::vector<RelationshipEdge> RelationshipStore::allEdges() const { return edges_; }
std::size_t RelationshipStore::size() const { return edges_.size(); }
void RelationshipStore::clear() { edges_.clear(); emittedEvents_.clear(); }

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

bool encodeCitizenMindRecord(const CitizenMindRecord& record,
                             std::vector<std::uint8_t>& out,
                             std::string* error,
                             bool includeCurrentThoughts) {
    if (record.schemaVersion != CitizenMindRecord::SchemaVersion) {
        setError(error, "unsupported citizen mind schema version");
        return false;
    }
    CitizenMindState state = record.state;
    CitizenMindSystem::normalize(state);
    if (!validateMindState(state, error)) return false;

    ByteWriter writer;
    writer.raw("ELYMIND1");
    writer.u32(record.schemaVersion);
    writer.u8(includeCurrentThoughts ? 1u : 0u);
    writer.u64(state.stableId);
    for (float v : state.personality.facets) writer.f32(v);
    for (float v : state.values.importance) writer.f32(v);

    writer.u32(static_cast<std::uint32_t>(state.preferences.entries.size()));
    for (const auto& pref : state.preferences.entries) {
        writer.u8(static_cast<std::uint8_t>(pref.category));
        writer.f32(pref.affinity);
        if (!writer.string(pref.key, error)) return false;
    }
    for (const auto& need : state.needs.dimensions) {
        writer.f32(need.strength);
        writer.f32(need.satisfaction);
    }
    writer.f32(state.stress.accumulated);
    writer.u8(static_cast<std::uint8_t>(state.stress.band));
    writer.u8(static_cast<std::uint8_t>(state.stress.previousBand));
    writer.i64(state.stress.bandChangedAt);
    writer.u8(static_cast<std::uint8_t>(state.stress.lastCrisisResponse));
    for (float v : state.coping.responseAffinity) writer.f32(v);

    if (includeCurrentThoughts) {
        writer.u32(static_cast<std::uint32_t>(state.thoughts.entries.size()));
        for (const auto& thought : state.thoughts.entries) if (!writeThought(writer, thought, error)) return false;
    } else {
        writer.u32(0u);
    }

    writer.u32(static_cast<std::uint32_t>(state.memories.entries.size()));
    for (const auto& memory : state.memories.entries) if (!writeMemory(writer, memory, error)) return false;

    out = std::move(writer.bytes);
    return true;
}

bool decodeCitizenMindRecord(std::span<const std::uint8_t> bytes,
                             CitizenMindRecord& out,
                             std::string* error) {
    ByteReader reader(bytes);
    CitizenMindRecord decoded{};
    std::uint8_t includeThoughts{};
    if (!reader.raw("ELYMIND1") || !reader.u32(decoded.schemaVersion)
        || decoded.schemaVersion != CitizenMindRecord::SchemaVersion
        || !reader.u8(includeThoughts) || includeThoughts > 1u
        || !reader.u64(decoded.state.stableId)) {
        setError(error, "invalid or unsupported citizen mind record header");
        return false;
    }
    for (float& v : decoded.state.personality.facets) if (!reader.f32(v)) { setError(error, "truncated personality data"); return false; }
    for (float& v : decoded.state.values.importance) if (!reader.f32(v)) { setError(error, "truncated value data"); return false; }

    std::uint32_t count{};
    if (!reader.u32(count) || count > 256u) { setError(error, "invalid preference count"); return false; }
    decoded.state.preferences.entries.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        PreferenceEntry pref{};
        std::uint8_t category{};
        if (!reader.u8(category) || category > static_cast<std::uint8_t>(PreferenceCategory::Other)
            || !reader.f32(pref.affinity) || !reader.string(pref.key)) {
            setError(error, "invalid preference payload");
            return false;
        }
        pref.category = static_cast<PreferenceCategory>(category);
        decoded.state.preferences.entries.push_back(std::move(pref));
    }
    for (auto& need : decoded.state.needs.dimensions) {
        if (!reader.f32(need.strength) || !reader.f32(need.satisfaction)) { setError(error, "truncated need data"); return false; }
    }
    std::uint8_t band{}, previousBand{}, response{};
    if (!reader.f32(decoded.state.stress.accumulated)
        || !reader.u8(band) || band > static_cast<std::uint8_t>(StressBand::Recovery)
        || !reader.u8(previousBand) || previousBand > static_cast<std::uint8_t>(StressBand::Recovery)
        || !reader.i64(decoded.state.stress.bandChangedAt)
        || !reader.u8(response) || response > static_cast<std::uint8_t>(CrisisResponse::Obsession)) {
        setError(error, "invalid stress payload");
        return false;
    }
    decoded.state.stress.band = static_cast<StressBand>(band);
    decoded.state.stress.previousBand = static_cast<StressBand>(previousBand);
    decoded.state.stress.lastCrisisResponse = static_cast<CrisisResponse>(response);
    for (float& v : decoded.state.coping.responseAffinity) if (!reader.f32(v)) { setError(error, "truncated coping payload"); return false; }

    if (!reader.u32(count) || count > CitizenMindSystem::kMaxCurrentThoughts
        || (includeThoughts == 0u && count != 0u)) {
        setError(error, "invalid thought count");
        return false;
    }
    decoded.state.thoughts.entries.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        ThoughtRecord thought{};
        if (!readThought(reader, thought)) { setError(error, "invalid thought payload"); return false; }
        decoded.state.thoughts.entries.push_back(std::move(thought));
    }
    if (!reader.u32(count) || count > CitizenMindSystem::kMaxMemories) { setError(error, "invalid memory count"); return false; }
    decoded.state.memories.entries.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        MemoryRecord memory{};
        if (!readMemory(reader, memory)) { setError(error, "invalid memory payload"); return false; }
        decoded.state.memories.entries.push_back(std::move(memory));
    }
    if (!reader.done()) { setError(error, "citizen mind record has trailing bytes"); return false; }

    CitizenMindSystem::normalize(decoded.state);
    if (!validateMindState(decoded.state, error)) return false;
    out = std::move(decoded);
    return true;
}

bool encodeRelationshipStore(const RelationshipStore& store,
                             std::vector<std::uint8_t>& out,
                             std::string* error) {
    const auto edges = store.allEdges();
    if (edges.size() > 1'000'000u) {
        setError(error, "relationship store exceeds persistence safety cap");
        return false;
    }
    ByteWriter writer;
    writer.raw("ELYREL01");
    writer.u32(1u);
    writer.u32(static_cast<std::uint32_t>(edges.size()));
    for (const auto& edge : edges) {
        if (!validateRelationshipEdge(edge, error)) return false;
        writer.u64(edge.source);
        writer.u64(edge.target);
        writer.u32(edge.typeTags);
        writer.f32(edge.affinity);
        writer.f32(edge.trust);
        writer.f32(edge.respect);
        writer.f32(edge.fear);
        writer.f32(edge.grievance);
        writer.f32(edge.familiarity);
        writer.i64(edge.lastInteraction);
        writer.u32(static_cast<std::uint32_t>(edge.eventRefs.size()));
        for (auto eventId : edge.eventRefs) writer.u64(eventId);
    }
    out = std::move(writer.bytes);
    return true;
}

bool decodeRelationshipStore(std::span<const std::uint8_t> bytes,
                             RelationshipStore& out,
                             std::string* error) {
    ByteReader reader(bytes);
    std::uint32_t schema{}, count{};
    if (!reader.raw("ELYREL01") || !reader.u32(schema) || schema != 1u
        || !reader.u32(count) || count > 1'000'000u) {
        setError(error, "invalid or unsupported relationship store header");
        return false;
    }
    RelationshipStore decoded;
    for (std::uint32_t i = 0; i < count; ++i) {
        RelationshipEdge edge{};
        std::uint32_t eventCount{};
        if (!reader.u64(edge.source) || !reader.u64(edge.target) || !reader.u32(edge.typeTags)
            || !reader.f32(edge.affinity) || !reader.f32(edge.trust) || !reader.f32(edge.respect)
            || !reader.f32(edge.fear) || !reader.f32(edge.grievance) || !reader.f32(edge.familiarity)
            || !reader.i64(edge.lastInteraction) || !reader.u32(eventCount)
            || eventCount > RelationshipStore::kMaxEventRefsPerEdge) {
            setError(error, "invalid relationship edge payload");
            return false;
        }
        edge.eventRefs.resize(eventCount);
        for (auto& eventId : edge.eventRefs) if (!reader.u64(eventId)) { setError(error, "truncated relationship event refs"); return false; }
        if (!decoded.upsert(std::move(edge), error)) return false;
    }
    if (!reader.done()) { setError(error, "relationship store has trailing bytes"); return false; }
    decoded.clearEmittedEvents();
    out = std::move(decoded);
    return true;
}

std::vector<std::uint8_t> encodeMindBatchCanonical(const std::vector<CitizenMindState>& states,
                                                   std::string* error) {
    std::vector<CitizenMindState> sorted = states;
    std::stable_sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.stableId < b.stableId; });
    ByteWriter writer;
    writer.raw("ELYMBAT1");
    writer.u32(static_cast<std::uint32_t>(sorted.size()));
    for (const auto& state : sorted) {
        std::vector<std::uint8_t> recordBytes;
        CitizenMindRecord record{};
        record.state = state;
        if (!encodeCitizenMindRecord(record, recordBytes, error, true)) return {};
        writer.u32(static_cast<std::uint32_t>(recordBytes.size()));
        writer.bytes.insert(writer.bytes.end(), recordBytes.begin(), recordBytes.end());
    }
    return writer.bytes;
}

} // namespace elysium
