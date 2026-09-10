#pragma once

#include "fortress/Components.hpp"

#include <string>
#include <vector>

namespace elysium::fortress {

struct CultureState {
    ContentId id;
    std::string name;
    std::vector<std::string> phonemes;
    Values medianValues{};
    std::vector<ContentId> favoredMaterials;
    std::vector<ContentId> artMotifs;
    std::vector<ContentId> cuisines;
    std::vector<ContentId> taboos;
};

struct LanguageState {
    ContentId id;
    std::string name;
    std::vector<std::string> onset;
    std::vector<std::string> nucleus;
    std::vector<std::string> coda;
};

std::string generateName(const LanguageState& language, std::uint64_t seed,
                         std::uint64_t identity, std::uint32_t syllables = 3);
float culturalCompatibility(const CultureState& a, const CultureState& b);

} // namespace elysium::fortress
