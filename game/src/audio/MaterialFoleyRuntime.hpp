#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::audio {
enum class FoleyEventKind:std::uint8_t{Mine,Impact,Footstep,Destroy,Break};
struct FoleyVariation{std::uint64_t assetContentId{};double minPitch{1},maxPitch{1},minIntensity{},maxIntensity{1};};
struct FoleySet{FoleyEventKind kind{};std::vector<FoleyVariation>variations;};
struct MaterialAudioFamily{std::uint64_t contentId{},materialClassId{};std::vector<FoleySet>sets;};
struct FoleyRequest{std::uint64_t eventStableId{},materialClassId{},actorStableId{};FoleyEventKind kind{};double intensity{1};std::uint64_t previousAssetId{};};
struct AudioEventPacket{std::uint64_t eventStableId{},assetContentId{},actorStableId{};double pitch{1},intensity{};bool fallback{};std::uint64_t voiceGroupId{};};
struct AudioVoiceBudget{std::uint64_t maxConcurrent{},currentlyActive{};};
class MaterialFoleyRuntime{public:bool publish(MaterialAudioFamily);void setFallback(std::uint64_t familyContentId);[[nodiscard]]AudioEventPacket resolve(const FoleyRequest&,const AudioVoiceBudget&)const;private:std::map<std::uint64_t,MaterialAudioFamily>familiesByMaterial_;std::uint64_t fallbackMaterial_{};};
} // namespace elysium::audio
