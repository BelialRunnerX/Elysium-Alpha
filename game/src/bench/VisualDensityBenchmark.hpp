#pragma once
#include <cstdint>
#include <map>
#include <string>
namespace elysium::bench {
enum class Evidence:std::uint8_t{FakeBackend,Headless,NativeRuntime,MeasuredHardware}; enum class Variant:std::uint8_t{Reference,ThinDetail,ReducedShadows,LodStress};
struct SceneConfig{std::uint64_t seed{};std::uint32_t version{1};Variant variant{Variant::Reference};};
struct Capture{std::uint64_t sceneFingerprint{};double editVisibleMs{};double generationMs{};double meshingMs{};double frameMs{};std::uint64_t triangles{},instances{},residentBytes{},queueDepth{};bool landmarkVisible{};Evidence evidence{Evidence::Headless};};
std::uint64_t fingerprint(SceneConfig) noexcept; Capture sanitize(SceneConfig,Capture); bool meaningPreserved(const Capture&) noexcept; std::string report(const Capture&);
}
