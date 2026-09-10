// Intended function: imported tools implementation for content_catalogue_dump; preserves the agent-authored subsystem contract for later integration/debugging.
#include "content/ContentCatalogue.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Row {
    std::string_view category;
    std::string_view id;
    std::string display;
};

template <typename Range, typename NameFn>
void appendRows(std::vector<Row>& rows, std::string_view category, const Range& range, NameFn&& nameFn) {
    for (const auto& value : range) {
        rows.push_back(Row{category, value.id, nameFn(value)});
    }
}

std::string sanitize(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (char ch : value) {
        if (ch == '\t' || ch == '\r' || ch == '\n') {
            result.push_back(' ');
        } else {
            result.push_back(ch);
        }
    }
    return result;
}

} // namespace

int main() {
    using namespace elysium::content;

    const auto errors = validateContentCatalogue();
    if (!errors.empty()) {
        std::cerr << "Agent 41 content catalogue validation failed:\n";
        for (const auto& error : errors) {
            std::cerr << " - " << error << '\n';
        }
        return 2;
    }

    std::vector<Row> rows;
    rows.reserve(sortedContentIds().size());

    appendRows(rows, "material", materials(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "block", blocks(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "construction_piece", constructionPieces(), [](const auto& v) { return std::string(v.family) + ": " + std::string(v.variant); });
    appendRows(rows, "planet_class", planetClasses(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "biome", biomes(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "weather", weather(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "ore", ores(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "item", items(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "machine", machines(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "recipe", recipes(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "race", races(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "class", classes(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "rune", runes(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "gear_material", gearMaterials(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "trinket", trinkets(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "crop", crops(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "food", foods(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "livestock", livestock(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "fauna_body_plan", faunaBodyPlans(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "fauna_trait", faunaTraitOptions(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "field_supply", fieldSupplies(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "ship_module", shipModules(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "vehicle", vehicles(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "faction", factions(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "enemy_role", enemyRoles(), [](const auto& v) { return std::string(v.family) + ": " + std::string(v.role); });
    appendRows(rows, "boss", bosses(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "enemy_variant", enemyVariants(), [](const auto& v) { return std::string(v.family) + ": " + std::string(v.variant); });
    appendRows(rows, "rift_horror_option", riftHorrorOptions(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "poi", pois(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "dungeon_room", dungeonRooms(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "settlement_template", settlementTemplates(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "institution_template", institutionTemplates(), [](const auto& v) { return std::string(v.name); });
    appendRows(rows, "contract", contracts(), [](const auto& v) { return std::string(v.type) + ": " + std::string(v.scope); });
    appendRows(rows, "court_envoy", courtEnvoys(), [](const auto& v) { return std::string(v.name); });

    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
        if (a.id != b.id) return a.id < b.id;
        return a.category < b.category;
    });

    const auto canonical = sortedContentIds();
    if (rows.size() != canonical.size()) {
        std::cerr << "Export row count does not match canonical ID count.\n";
        return 3;
    }
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].id != canonical[i]) {
            std::cerr << "Export ordering diverges from canonical stable ID order at index " << i << ".\n";
            return 4;
        }
    }

    std::cout << "# agent\t41\n";
    std::cout << "# catalogue_fingerprint\t" << catalogueFingerprint() << "\n";
    std::cout << "# stable_id_count\t" << rows.size() << "\n";
    std::cout << "category\tstable_id\tdisplay_name\n";
    for (const auto& row : rows) {
        std::cout << row.category << '\t' << row.id << '\t' << sanitize(row.display) << '\n';
    }
    return 0;
}
