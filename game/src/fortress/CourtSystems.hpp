#pragma once

#include "fortress/Components.hpp"

#include <span>
#include <string_view>

namespace elysium::fortress {

enum class CourtMetric : std::uint8_t { Favor, Suspicion, Always };

struct CourtEnvoyDefinition {
    std::string_view id;
    std::string_view name;
    std::string_view office;
    CourtMetric metric{CourtMetric::Favor};
    float minimum{};
    float maximum{100.0f};
};

struct CourtOffer {
    StableId id{};
    ContentId envoy;
    ContentId request;
    ContentId reward;
    float tributeValue{};
    float standingDelta{};
    std::uint64_t system{};
    bool accepted{};
    bool completed{};
};

std::span<const CourtEnvoyDefinition> courtEnvoys();
bool envoyEligible(const CourtEnvoyDefinition& envoy, float favor, float suspicion);
CourtOffer rollCourtOffer(std::uint64_t seed, std::uint64_t epoch, const CourtEnvoyDefinition& envoy,
                         std::uint64_t system, float favor, float suspicion);

} // namespace elysium::fortress
