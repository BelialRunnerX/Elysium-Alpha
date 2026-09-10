#pragma once

#include "fortress/Common.hpp"

#include <vector>

namespace elysium::fortress {

enum class GeologicalLayerKind : std::uint8_t {
    SurfaceRegolith,
    HostRock,
    AquiferVolatile,
    CavernEcosystem,
    DeepResource,
    MantleFloor
};

struct GeologicalLayer {
    GeologicalLayerKind kind{GeologicalLayerKind::HostRock};
    ContentId material;
    float topAltitude{};
    float bottomAltitude{};
    float stability{1.0f};
    float permeability{};
    float heat{};
    float radiation{};
    ContentId volatileMedium;
};

struct GeologicalColumn {
    CellAddress surfaceAddress{};
    std::vector<GeologicalLayer> layers;
};

GeologicalColumn generateGeologicalColumn(std::uint64_t seed, const CellAddress& address,
                                          ContentId planetClass);
float foundationSuitability(const GeologicalColumn& column, float requestedAltitude);

} // namespace elysium::fortress
