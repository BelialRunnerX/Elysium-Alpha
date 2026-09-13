// Intended function: imported core implementation for ContentExtensions; preserves the agent-authored subsystem contract for later integration/debugging.
#include "core/ContentExtensions.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <limits>
#include <sstream>
#include <tuple>

namespace elysium {
namespace {

void setError(std::string* error, std::string message) {
    if (error) *error = std::move(message);
}

bool validSegmentChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.';
}

bool validPath(std::string_view path) {
    if (path.empty() || path.front() == '/' || path.back() == '/') return false;
    bool haveChar = false;
    for (char c : path) {
        if (c == '/') {
            if (!haveChar) return false;
            haveChar = false;
            continue;
        }
        if (!validSegmentChar(c)) return false;
        haveChar = true;
    }
    return haveChar;
}

std::uint64_t stableStringHash(std::string_view value) {
    // Fixed FNV-1a byte pass, mixed once through the project's stable mix. This
    // is an explicit protocol helper, unlike implementation-defined std::hash.
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : value) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }
    return mix64(h);
}

bool contentDescriptorLess(const ContentDescriptor& a, const ContentDescriptor& b) {
    return a.id < b.id;
}

bool schemaLess(const ExtensionSchemaDescriptor& a, const ExtensionSchemaDescriptor& b) {
    return a.objectType < b.objectType;
}

void hashCombine(std::uint64_t& h, std::uint64_t v) {
    h = mix64(h ^ mix64(v));
}

std::string hexEncode(std::string_view input) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out;
    out.resize(input.size() * 2);
    for (std::size_t i = 0; i < input.size(); ++i) {
        const auto c = static_cast<unsigned char>(input[i]);
        out[i * 2] = digits[c >> 4U];
        out[i * 2 + 1] = digits[c & 0x0FU];
    }
    return out;
}

std::optional<std::string> hexDecode(std::string_view input) {
    if ((input.size() & 1U) != 0U) return std::nullopt;
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    std::string out;
    out.resize(input.size() / 2);
    for (std::size_t i = 0; i < out.size(); ++i) {
        const int hi = nibble(input[i * 2]);
        const int lo = nibble(input[i * 2 + 1]);
        if (hi < 0 || lo < 0) return std::nullopt;
        out[i] = static_cast<char>((hi << 4) | lo);
    }
    return out;
}

bool parseU64(std::string_view text, std::uint64_t& out) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const auto result = std::from_chars(begin, end, out);
    return result.ec == std::errc{} && result.ptr == end;
}

bool parseU32(std::string_view text, std::uint32_t& out) {
    std::uint64_t temp{};
    if (!parseU64(text, temp) || temp > std::numeric_limits<std::uint32_t>::max()) return false;
    out = static_cast<std::uint32_t>(temp);
    return true;
}

std::vector<std::string_view> splitPipe(std::string_view line) {
    std::vector<std::string_view> fields;
    std::size_t start = 0;
    while (true) {
        const auto pos = line.find('|', start);
        if (pos == std::string_view::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    return fields;
}

} // namespace

ContentId::ContentId(std::string canonical) : value_(std::move(canonical)) {}

std::optional<ContentId> ContentId::parse(std::string_view text, std::string* error) {
    if (!valid(text)) {
        setError(error, "content id must be canonical lowercase namespace:path using [a-z0-9_.-/]");
        return std::nullopt;
    }
    return ContentId(std::string(text));
}

bool ContentId::valid(std::string_view text) {
    const auto colon = text.find(':');
    if (colon == std::string_view::npos || colon == 0 || colon + 1 >= text.size()) return false;
    if (text.find(':', colon + 1) != std::string_view::npos) return false;
    const auto ns = text.substr(0, colon);
    const auto path = text.substr(colon + 1);
    if (!std::all_of(ns.begin(), ns.end(), validSegmentChar)) return false;
    return validPath(path);
}

std::uint64_t ContentId::stableHash() const {
    return stableStringHash(value_);
}

bool ContentRegistry::registerContent(ContentDescriptor descriptor, std::string* error) {
    if (frozen_) {
        setError(error, "content registry is frozen; late registration rejected");
        return false;
    }
    if (!ContentId::valid(descriptor.id.str())) {
        setError(error, "content descriptor has invalid namespaced id");
        return false;
    }
    if (descriptor.definitionVersion == 0) {
        setError(error, "content descriptor definitionVersion must be non-zero");
        return false;
    }
    pending_.push_back(std::move(descriptor));
    return true;
}

bool ContentRegistry::freeze(std::string* error) {
    if (frozen_) return true;
    ordered_ = pending_;
    std::sort(ordered_.begin(), ordered_.end(), contentDescriptorLess);
    for (std::size_t i = 1; i < ordered_.size(); ++i) {
        if (ordered_[i - 1].id == ordered_[i].id) {
            setError(error, "duplicate content id during registry freeze: " + ordered_[i].id.str());
            ordered_.clear();
            return false;
        }
    }
    pending_.clear();
    frozen_ = true;
    return true;
}

ResolvedContentRef ContentRegistry::resolve(const ContentId& id) const {
    const auto it = std::lower_bound(ordered_.begin(), ordered_.end(), id,
        [](const ContentDescriptor& descriptor, const ContentId& key) { return descriptor.id < key; });
    if (it != ordered_.end() && it->id == id) return {id, ContentResolution::Resolved, &*it};
    return {id, ContentResolution::UnresolvedPlaceholder, nullptr};
}

std::optional<std::size_t> ContentRegistry::runtimeOrdinal(const ContentId& id) const {
    const auto it = std::lower_bound(ordered_.begin(), ordered_.end(), id,
        [](const ContentDescriptor& descriptor, const ContentId& key) { return descriptor.id < key; });
    if (it == ordered_.end() || !(it->id == id)) return std::nullopt;
    return static_cast<std::size_t>(std::distance(ordered_.begin(), it));
}

std::uint64_t ContentRegistry::manifestFingerprint() const {
    std::uint64_t h = mix64(0x454C595349554D43ULL); // "ELYSIUMC"
    for (const auto& descriptor : ordered_) {
        hashCombine(h, descriptor.id.stableHash());
        hashCombine(h, static_cast<std::uint64_t>(descriptor.extensionPoint));
        hashCombine(h, descriptor.definitionVersion);
        hashCombine(h, descriptor.deterministicFingerprint);
    }
    return h;
}

std::vector<ContentManifestEntry> captureContentManifest(const ContentRegistry& registry) {
    std::vector<ContentManifestEntry> out;
    out.reserve(registry.ordered().size());
    for (const auto& descriptor : registry.ordered()) {
        out.push_back({descriptor.id, descriptor.definitionVersion, descriptor.deterministicFingerprint});
    }
    return out;
}

ContentManifestCompatibility compareContentManifest(const std::vector<ContentManifestEntry>& saved,
                                                    const ContentRegistry& current) {
    ContentManifestCompatibility result;
    for (const auto& entry : saved) {
        const auto resolved = current.resolve(entry.id);
        if (resolved.resolution == ContentResolution::UnresolvedPlaceholder) {
            result.missingIds.push_back(entry.id);
            continue;
        }
        if (resolved.descriptor->definitionVersion != entry.definitionVersion ||
            resolved.descriptor->deterministicFingerprint != entry.deterministicFingerprint) {
            result.changedDefinitions.push_back(entry.id);
        }
    }
    std::sort(result.missingIds.begin(), result.missingIds.end());
    std::sort(result.changedDefinitions.begin(), result.changedDefinitions.end());
    result.compatibleWithoutMigration = result.changedDefinitions.empty();
    return result;
}

bool ExtensionSchemaRegistry::registerSchema(ExtensionSchemaDescriptor descriptor, std::string* error) {
    if (frozen_) {
        setError(error, "extension schema registry is frozen; late registration rejected");
        return false;
    }
    if (!ContentId::valid(descriptor.objectType.str())) {
        setError(error, "extension schema has invalid namespaced object type");
        return false;
    }
    if (descriptor.currentVersion == 0 || descriptor.oldestReadableVersion == 0 ||
        descriptor.oldestReadableVersion > descriptor.currentVersion) {
        setError(error, "extension schema version range is invalid");
        return false;
    }
    pending_.push_back(std::move(descriptor));
    return true;
}

bool ExtensionSchemaRegistry::freeze(std::string* error) {
    if (frozen_) return true;
    ordered_ = pending_;
    std::sort(ordered_.begin(), ordered_.end(), schemaLess);
    for (std::size_t i = 1; i < ordered_.size(); ++i) {
        if (ordered_[i - 1].objectType == ordered_[i].objectType) {
            setError(error, "duplicate extension object schema: " + ordered_[i].objectType.str());
            ordered_.clear();
            return false;
        }
    }
    pending_.clear();
    frozen_ = true;
    return true;
}

const ExtensionSchemaDescriptor* ExtensionSchemaRegistry::find(const ContentId& type) const {
    const auto it = std::lower_bound(ordered_.begin(), ordered_.end(), type,
        [](const ExtensionSchemaDescriptor& descriptor, const ContentId& key) { return descriptor.objectType < key; });
    return (it != ordered_.end() && it->objectType == type) ? &*it : nullptr;
}

bool ExtensionSchemaRegistry::canRead(const ContentId& type, std::uint32_t version) const {
    const auto* schema = find(type);
    return schema && version >= schema->oldestReadableVersion && version <= schema->currentVersion;
}

bool validatePersistentExtensionObject(const PersistentExtensionObject& object, std::string* error) {
    if (object.stableObjectId == 0) {
        setError(error, "persistent extension object requires non-zero stableObjectId");
        return false;
    }
    if (!ContentId::valid(object.objectType.str())) {
        setError(error, "persistent extension object has invalid object type id");
        return false;
    }
    if (object.schemaVersion == 0) {
        setError(error, "persistent extension object schemaVersion must be non-zero");
        return false;
    }
    return true;
}

std::string serializePersistentExtensionObject(const PersistentExtensionObject& object) {
    if (!validatePersistentExtensionObject(object)) return {};
    std::ostringstream out;
    out << "ELYSIUM_EXT_OBJECT|1|" << object.stableObjectId << '|' << object.objectType.str() << '|'
        << object.schemaVersion << '|' << hexEncode(object.payload);
    return out.str();
}

std::optional<PersistentExtensionObject> deserializePersistentExtensionObject(std::string_view text, std::string* error) {
    const auto fields = splitPipe(text);
    if (fields.size() != 6 || fields[0] != "ELYSIUM_EXT_OBJECT" || fields[1] != "1") {
        setError(error, "invalid persistent extension object envelope");
        return std::nullopt;
    }
    PersistentExtensionObject object;
    if (!parseU64(fields[2], object.stableObjectId)) {
        setError(error, "invalid persistent extension object stable id");
        return std::nullopt;
    }
    auto type = ContentId::parse(fields[3], error);
    if (!type) return std::nullopt;
    object.objectType = std::move(*type);
    if (!parseU32(fields[4], object.schemaVersion)) {
        setError(error, "invalid persistent extension object schema version");
        return std::nullopt;
    }
    auto payload = hexDecode(fields[5]);
    if (!payload) {
        setError(error, "invalid persistent extension object payload encoding");
        return std::nullopt;
    }
    object.payload = std::move(*payload);
    if (!validatePersistentExtensionObject(object, error)) return std::nullopt;
    return object;
}

bool ModCommandBuffer::submit(ModMutationCommand command, std::string* error) {
    if (!ContentId::valid(command.originMod.str()) || !ContentId::valid(command.payloadSchema.str())) {
        setError(error, "mod command requires canonical origin mod and payload schema ids");
        return false;
    }
    if (command.originSequence == 0 || command.payloadSchemaVersion == 0) {
        setError(error, "mod command sequence/schema version must be non-zero");
        return false;
    }
    const auto duplicate = std::find_if(pending_.begin(), pending_.end(), [&](const ModMutationCommand& existing) {
        return existing.originMod == command.originMod && existing.originSequence == command.originSequence;
    });
    if (duplicate != pending_.end()) {
        setError(error, "duplicate mod command origin sequence rejected");
        return false;
    }
    pending_.push_back(std::move(command));
    return true;
}

std::vector<ModMutationCommand> ModCommandBuffer::drainDeterministic() {
    std::sort(pending_.begin(), pending_.end(), [](const ModMutationCommand& a, const ModMutationCommand& b) {
        return std::tie(a.originMod, a.originSequence, a.kind, a.stableTargetId, a.payloadSchema) <
               std::tie(b.originMod, b.originSequence, b.kind, b.stableTargetId, b.payloadSchema);
    });
    std::vector<ModMutationCommand> out;
    out.swap(pending_);
    return out;
}

bool validateReplicationRecord(const ReplicationRecord& record, std::string* error) {
    if (record.streamStableId == 0 || record.sequence == 0) {
        setError(error, "replication record requires non-zero stable stream id and sequence");
        return false;
    }
    if (!ContentId::valid(record.payloadSchema.str()) || record.payloadSchemaVersion == 0) {
        setError(error, "replication record requires canonical versioned payload schema");
        return false;
    }
    if ((record.kind == ReplicationRecordKind::ObjectUpsert || record.kind == ReplicationRecordKind::ObjectTombstone ||
         record.kind == ReplicationRecordKind::AuthoritativeEntityState) && record.objectStableId == 0) {
        setError(error, "object-scoped replication record requires stable object id");
        return false;
    }
    return true;
}

std::string serializeReplicationRecord(const ReplicationRecord& record) {
    if (!validateReplicationRecord(record)) return {};
    std::ostringstream out;
    out << "ELYSIUM_REPL|1|" << static_cast<unsigned>(record.kind) << '|' << record.streamStableId << '|'
        << record.sequence << '|' << record.objectStableId << '|' << record.payloadSchema.str() << '|'
        << record.payloadSchemaVersion << '|' << hexEncode(record.payload);
    return out.str();
}

std::optional<ReplicationRecord> deserializeReplicationRecord(std::string_view text, std::string* error) {
    const auto fields = splitPipe(text);
    if (fields.size() != 9 || fields[0] != "ELYSIUM_REPL" || fields[1] != "1") {
        setError(error, "invalid replication record envelope");
        return std::nullopt;
    }
    std::uint64_t kindValue{};
    ReplicationRecord record;
    if (!parseU64(fields[2], kindValue) || kindValue > static_cast<std::uint64_t>(ReplicationRecordKind::AuthoritativeEntityState)) {
        setError(error, "invalid replication record kind");
        return std::nullopt;
    }
    record.kind = static_cast<ReplicationRecordKind>(kindValue);
    if (!parseU64(fields[3], record.streamStableId) || !parseU64(fields[4], record.sequence) ||
        !parseU64(fields[5], record.objectStableId)) {
        setError(error, "invalid replication stable identifiers");
        return std::nullopt;
    }
    auto schema = ContentId::parse(fields[6], error);
    if (!schema) return std::nullopt;
    record.payloadSchema = std::move(*schema);
    if (!parseU32(fields[7], record.payloadSchemaVersion)) {
        setError(error, "invalid replication schema version");
        return std::nullopt;
    }
    auto payload = hexDecode(fields[8]);
    if (!payload) {
        setError(error, "invalid replication payload encoding");
        return std::nullopt;
    }
    record.payload = std::move(*payload);
    if (!validateReplicationRecord(record, error)) return std::nullopt;
    return record;
}

} // namespace elysium
