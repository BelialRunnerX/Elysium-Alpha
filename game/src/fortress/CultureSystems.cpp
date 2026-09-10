#include "fortress/CultureSystems.hpp"

#include <algorithm>
#include <cctype>

namespace elysium::fortress {
namespace {
const std::string& pick(const std::vector<std::string>& values, std::uint64_t token) {
    static const std::string empty;
    if (values.empty()) return empty;
    return values[token % values.size()];
}
}

std::string generateName(const LanguageState& language, std::uint64_t seed,
                         std::uint64_t identity, std::uint32_t syllables) {
    std::string name;
    const std::uint32_t count = std::clamp(syllables, 1U, 8U);
    for (std::uint32_t i = 0; i < count; ++i) {
        const auto token = deterministicToken(seed, identity, 0x4E414D4500000000ULL + i, count);
        name += pick(language.onset, token);
        name += pick(language.nucleus, token >> 11U);
        name += pick(language.coda, token >> 23U);
    }
    if (!name.empty()) name.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(name.front())));
    return name;
}

float culturalCompatibility(const CultureState& a, const CultureState& b) {
    const auto diff = [](float x, float y) { return std::abs(x - y); };
    float valueDistance = diff(a.medianValues.family, b.medianValues.family) +
                          diff(a.medianValues.law, b.medianValues.law) +
                          diff(a.medianValues.independence, b.medianValues.independence) +
                          diff(a.medianValues.empire, b.medianValues.empire) +
                          diff(a.medianValues.tradition, b.medianValues.tradition) +
                          diff(a.medianValues.craft, b.medianValues.craft) +
                          diff(a.medianValues.knowledge, b.medianValues.knowledge) +
                          diff(a.medianValues.nature, b.medianValues.nature) +
                          diff(a.medianValues.wealth, b.medianValues.wealth) +
                          diff(a.medianValues.martialHonor, b.medianValues.martialHonor);
    valueDistance /= 10.0f;
    std::size_t sharedMotifs{};
    for (const auto& motif : a.artMotifs) if (std::find(b.artMotifs.begin(), b.artMotifs.end(), motif) != b.artMotifs.end()) ++sharedMotifs;
    const float motifAffinity = a.artMotifs.empty() ? 0.5f : saturate(static_cast<float>(sharedMotifs) / static_cast<float>(a.artMotifs.size()));
    return saturate((1.0f - valueDistance) * 0.8f + motifAffinity * 0.2f);
}

} // namespace elysium::fortress
