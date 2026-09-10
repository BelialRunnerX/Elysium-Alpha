#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::audio {
enum class AudioLayerKind:std::uint8_t{Wind,SuitTick,Acid,Radiation,PressureCreak,ImperialMachinery,FrontierActivity,ExplorationMusic,SuspicionMusic,RegisterActionMusic};
enum class VisualTwinKind:std::uint8_t{HazardIndicator,PressureWarning,RadiationWarning,RegisterActionWarning};
struct AudioWorldSnapshot{std::uint64_t tick{},locationStableId{},factionPaletteId{};double wind{},acid{},radiation{},pressureStress{},suspicion{};bool claimSafe{},registerActionActive{},dialogueActive{};};
struct AudioLayerPacket{AudioLayerKind kind{};std::uint64_t paletteId{};double intensity{},duck{};bool continuous{};};
struct VisualCueEvent{VisualTwinKind kind{};double intensity{};};
struct AudioDirectorFrame{std::vector<AudioLayerPacket>layers;std::vector<VisualCueEvent>visualTwins;};
struct AudioDirectorPolicy{double engageThreshold{0.08},releaseThreshold{0.04};std::uint64_t cooldownTicks{30};double dialogueDuck{0.35};};
class WorldAudioDirector{public:[[nodiscard]]AudioDirectorFrame update(const AudioWorldSnapshot&,const AudioDirectorPolicy& policy = {});private:struct LayerState{bool active{};std::uint64_t lastChangeTick{};};std::map<AudioLayerKind,LayerState>state_;};
} // namespace elysium::audio
