#include "fortress/GeologySystems.hpp"

#include <algorithm>

namespace elysium::fortress {

GeologicalColumn generateGeologicalColumn(std::uint64_t seed, const CellAddress& address,
                                          ContentId planetClass) {
    GeologicalColumn column{};
    column.surfaceAddress = address;
    const auto token = hashCoords(seed ^ address.planet, address.u, address.v, address.radial,
                                  0x47454F4C4F4759ULL + address.face);
    const float surfaceThickness = 2.0f + static_cast<float>(token & 0xFU) * 0.35f;
    const float aquiferDepth = 40.0f + static_cast<float>((token >> 8U) & 0x7FU) * 1.5f;
    const float cavernDepth = aquiferDepth + 80.0f + static_cast<float>((token >> 20U) & 0x7FU) * 1.8f;
    const bool toxic = planetClass.value.find("toxic") != std::string::npos;
    const bool frozen = planetClass.value.find("frozen") != std::string::npos;
    const bool scorched = planetClass.value.find("scorched") != std::string::npos;
    const bool irradiated = planetClass.value.find("irradiated") != std::string::npos;

    column.layers.push_back({GeologicalLayerKind::SurfaceRegolith, ContentId{"elysium:block/regolith"}, 256.0f, 256.0f - surfaceThickness, 0.45f, 0.45f, 0.1f, 0.0f, {}});
    column.layers.push_back({GeologicalLayerKind::HostRock, ContentId{scorched ? "elysium:block/basalt" : "elysium:block/base_stone"}, 256.0f - surfaceThickness, -aquiferDepth, 0.8f, 0.08f, scorched ? 0.55f : 0.2f, irradiated ? 0.25f : 0.02f, {}});
    column.layers.push_back({GeologicalLayerKind::AquiferVolatile, ContentId{"elysium:block/fractured_stone"}, -aquiferDepth, -cavernDepth + 25.0f, 0.55f, 0.65f, scorched ? 0.75f : frozen ? 0.05f : 0.25f, irradiated ? 0.4f : 0.05f,
                             ContentId{toxic ? "elysium:medium/acid" : frozen ? "elysium:medium/cryofluid" : "elysium:medium/water"}});
    column.layers.push_back({GeologicalLayerKind::CavernEcosystem, ContentId{"elysium:block/deep_stone"}, -cavernDepth + 25.0f, -cavernDepth - 120.0f, 0.5f, 0.25f, 0.35f, irradiated ? 0.5f : 0.1f, {}});
    column.layers.push_back({GeologicalLayerKind::DeepResource, ContentId{"elysium:block/deep_stone"}, -cavernDepth - 120.0f, -500.0f, 0.9f, 0.03f, 0.7f, irradiated ? 0.8f : 0.2f, {}});
    column.layers.push_back({GeologicalLayerKind::MantleFloor, ContentId{"elysium:block/mantle_shell"}, -500.0f, -512.0f, 1.0f, 0.0f, 1.0f, 0.5f, {}});
    return column;
}

float foundationSuitability(const GeologicalColumn& column, float requestedAltitude) {
    for (const auto& layer : column.layers) {
        if (requestedAltitude <= layer.topAltitude && requestedAltitude >= layer.bottomAltitude) {
            return saturate(layer.stability * (1.0f - layer.permeability * 0.35f) * (1.0f - layer.heat * 0.1f));
        }
    }
    return 0.25f;
}

} // namespace elysium::fortress
