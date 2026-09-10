// Intended function: imported ecs implementation for CitizenPersistence; preserves the agent-authored subsystem contract for later integration/debugging.
#include "ecs/CitizenPersistence.hpp"

#include "ecs/CitizenLifecycle.hpp"

#include <iomanip>
#include <limits>
#include <sstream>

namespace elysium {
namespace {

constexpr const char* kMagic = "ELYSIUM_CITIZEN_ENTITY";

void writeIds(std::ostringstream& out, const char* tag, const std::vector<CitizenStableId>& ids) {
    out << tag << ' ' << ids.size();
    for (const auto id : ids) out << ' ' << id;
    out << '\n';
}

bool readIds(std::istringstream& in, const char* expected, std::vector<CitizenStableId>& ids, std::string* error) {
    std::string tag;
    std::size_t count{};
    if (!(in >> tag >> count) || tag != expected) {
        if (error) *error = std::string("expected ") + expected + " list";
        return false;
    }
    if (count > 1'000'000) {
        if (error) *error = std::string(expected) + " list is unreasonably large";
        return false;
    }
    ids.clear();
    ids.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        CitizenStableId id{};
        if (!(in >> id)) {
            if (error) *error = std::string("truncated ") + expected + " list";
            return false;
        }
        ids.push_back(id);
    }
    return true;
}

void writeCapabilities(std::ostringstream& out, const char* tag, const CitizenCapabilityValues& values) {
    out << tag;
    for (const auto value : values) out << ' ' << value;
    out << '\n';
}

bool readCapabilities(std::istringstream& in, const char* expected, CitizenCapabilityValues& values, std::string* error) {
    std::string tag;
    if (!(in >> tag) || tag != expected) {
        if (error) *error = std::string("expected ") + expected + " capability row";
        return false;
    }
    for (auto& value : values) {
        if (!(in >> value)) {
            if (error) *error = std::string("truncated ") + expected + " capability row";
            return false;
        }
    }
    return true;
}

template<class Enum>
bool readEnum(std::istringstream& in, Enum& out) {
    int raw{};
    if (!(in >> raw)) return false;
    out = static_cast<Enum>(raw);
    return true;
}

bool readCommonBody(std::istringstream& in, RemoteCitizenRecord& record, std::string* error) {
    std::string tag;
    if (!(in >> tag >> record.persistentId.value) || tag != "id") {
        if (error) *error = "missing citizen stable ID";
        return false;
    }

    int named{};
    if (!(in >> tag) || tag != "identity" ||
        !(in >> std::quoted(record.identity.name) >> std::quoted(record.identity.speciesRef)
             >> std::quoted(record.identity.raceRef) >> named)) {
        if (error) *error = "invalid citizen identity row";
        return false;
    }
    record.identity.named = named != 0;

    if (!(in >> tag) || tag != "membership" ||
        !(in >> record.membership.homeSettlementId >> record.membership.currentSettlementId
             >> record.membership.primaryFactionId) || !readEnum(in, record.membership.citizenship)) {
        if (error) *error = "invalid citizen membership row";
        return false;
    }
    if (!readIds(in, "organizations", record.membership.organizationIds, error)) return false;

    if (!(in >> tag) || tag != "life" || !(in >> record.lifeStage.ageYears >> record.lifeStage.birthTime) ||
        !readEnum(in, record.lifeStage.stage)) {
        if (error) *error = "invalid citizen life row";
        return false;
    }

    int conscious{}, alive{};
    if (!(in >> tag) || tag != "biological" ||
        !(in >> record.biological.nutrition01 >> record.biological.rest01 >> record.biological.respiration01
             >> conscious >> alive)) {
        if (error) *error = "invalid citizen biological row";
        return false;
    }
    record.biological.conscious = conscious != 0;
    record.biological.alive = alive != 0;

    if (!(in >> tag) || tag != "body" ||
        !(in >> std::quoted(record.body.bodyPlanRef) >> record.body.bodyStateRef)) {
        if (error) *error = "invalid citizen body row";
        return false;
    }
    if (!(in >> tag) || tag != "profession" || !(in >> std::quoted(record.profession.professionRef))) {
        if (error) *error = "invalid citizen profession row";
        return false;
    }
    if (!readCapabilities(in, "capabilities", record.laborCapabilities.base, error) ||
        !readCapabilities(in, "impairment", record.impairment.multiplier, error)) return false;
    if (!readIds(in, "owned", record.possessions.ownedUniqueItemIds, error) ||
        !readIds(in, "equipped", record.possessions.equippedItemIds, error)) return false;
    if (!(in >> tag >> record.possessions.quartersId) || tag != "quarters") {
        if (error) *error = "invalid citizen quarters row";
        return false;
    }
    if (!readIds(in, "offices", record.offices.officeIds, error)) return false;
    if (!(in >> tag >> record.military.squadId >> record.military.uniformProfileId) || tag != "military") {
        if (error) *error = "invalid citizen military row";
        return false;
    }
    if (!readIds(in, "relationships", record.biography.relationshipIds, error) ||
        !readIds(in, "memories", record.biography.importantMemoryIds, error) ||
        !readIds(in, "history", record.biography.historyEventIds, error)) return false;
    return true;
}

bool readPersistence(std::istringstream& in, RemoteCitizenRecord& record, std::string* error) {
    std::string tag;
    int representation{}, historical{}, domestic{}, allowCohort{};
    if (!(in >> tag >> representation >> historical >> domestic >> allowCohort >> record.persistence.lastStrategicUpdate) ||
        tag != "persistence") {
        if (error) *error = "invalid citizen persistence row";
        return false;
    }
    record.persistence.representation = static_cast<CitizenRepresentation>(representation);
    record.persistence.historicalSignificance = historical != 0;
    record.persistence.namedDomestic = domestic != 0;
    record.persistence.allowRemoteCohort = allowCohort != 0;
    return true;
}

} // namespace

std::string serializeRemoteCitizenRecord(const RemoteCitizenRecord& record) {
    std::string validation;
    if (!CitizenLifecycleSystem::validate(record, &validation))
        throw std::invalid_argument("cannot serialize invalid citizen record: " + validation);

    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << kMagic << ' ' << kCitizenEntitySchemaVersion << '\n';
    out << "id " << record.persistentId.value << '\n';
    out << "identity " << std::quoted(record.identity.name) << ' '
        << std::quoted(record.identity.speciesRef) << ' '
        << std::quoted(record.identity.raceRef) << ' ' << (record.identity.named ? 1 : 0) << '\n';
    out << "membership " << record.membership.homeSettlementId << ' '
        << record.membership.currentSettlementId << ' ' << record.membership.primaryFactionId << ' '
        << static_cast<int>(record.membership.citizenship) << '\n';
    writeIds(out, "organizations", record.membership.organizationIds);
    out << "life " << record.lifeStage.ageYears << ' ' << record.lifeStage.birthTime << ' '
        << static_cast<int>(record.lifeStage.stage) << '\n';
    out << "biological " << record.biological.nutrition01 << ' ' << record.biological.rest01 << ' '
        << record.biological.respiration01 << ' ' << (record.biological.conscious ? 1 : 0) << ' '
        << (record.biological.alive ? 1 : 0) << '\n';
    out << "body " << std::quoted(record.body.bodyPlanRef) << ' ' << record.body.bodyStateRef << '\n';
    out << "profession " << std::quoted(record.profession.professionRef) << '\n';
    writeCapabilities(out, "capabilities", record.laborCapabilities.base);
    writeCapabilities(out, "impairment", record.impairment.multiplier);
    writeIds(out, "owned", record.possessions.ownedUniqueItemIds);
    writeIds(out, "equipped", record.possessions.equippedItemIds);
    out << "quarters " << record.possessions.quartersId << '\n';
    writeIds(out, "offices", record.offices.officeIds);
    out << "military " << record.military.squadId << ' ' << record.military.uniformProfileId << '\n';
    writeIds(out, "relationships", record.biography.relationshipIds);
    writeIds(out, "memories", record.biography.importantMemoryIds);
    writeIds(out, "history", record.biography.historyEventIds);
    out << "location " << record.strategicLocation.systemId << ' '
        << record.strategicLocation.planetId << ' ' << record.strategicLocation.siteId << '\n';
    out << "shard " << static_cast<int>(record.shardState.shard) << ' '
        << record.shardState.transitionSerial << '\n';
    out << "persistence " << static_cast<int>(record.persistence.representation) << ' '
        << (record.persistence.historicalSignificance ? 1 : 0) << ' '
        << (record.persistence.namedDomestic ? 1 : 0) << ' '
        << (record.persistence.allowRemoteCohort ? 1 : 0) << ' '
        << record.persistence.lastStrategicUpdate << '\n';
    return out.str();
}

std::optional<RemoteCitizenRecord> deserializeRemoteCitizenRecord(std::string_view text, std::string* error) {
    std::istringstream in{std::string(text)};
    std::string magic;
    std::uint32_t schema{};
    if (!(in >> magic >> schema) || magic != kMagic) {
        if (error) *error = "invalid citizen entity magic";
        return std::nullopt;
    }
    if (schema < kCitizenEntityOldestReadableSchemaVersion || schema > kCitizenEntitySchemaVersion) {
        if (error) *error = "unsupported citizen entity schema version";
        return std::nullopt;
    }

    RemoteCitizenRecord record{};
    record.schemaVersion = schema;
    if (!readCommonBody(in, record, error)) return std::nullopt;

    std::string tag;
    if (schema >= 2) {
        if (!(in >> tag >> record.strategicLocation.systemId >> record.strategicLocation.planetId
                 >> record.strategicLocation.siteId) || tag != "location") {
            if (error) *error = "invalid citizen strategic location row";
            return std::nullopt;
        }
        if (!(in >> tag) || tag != "shard" || !readEnum(in, record.shardState.shard) ||
            !(in >> record.shardState.transitionSerial)) {
            if (error) *error = "invalid citizen shard row";
            return std::nullopt;
        }
    } else {
        // Schema v1 predates explicit strategic location/shard metadata. Current
        // settlement is the only durable site identity it carried, so migrate it
        // conservatively and derive a remote shard from the stored representation.
        record.strategicLocation.siteId = record.membership.currentSettlementId;
    }

    if (!readPersistence(in, record, error)) return std::nullopt;

    if (schema == 1) {
        record.shardState.shard = record.persistence.representation == CitizenRepresentation::Active
            ? CitizenSimulationShard::ActiveFortress
            : CitizenSimulationShard::ActivePlanetRemote;
        record.shardState.transitionSerial = 0;
    }

    std::string validation;
    if (!CitizenLifecycleSystem::validate(record, &validation)) {
        if (error) *error = validation;
        return std::nullopt;
    }
    return record;
}

} // namespace elysium
