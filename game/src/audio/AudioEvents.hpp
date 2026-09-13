// Intended function: presentation-only prioritized audio event queue for impacts, machines, UI, ambience, alerts and dialogue.
#pragma once
#include <cstdint>
#include <vector>
namespace elysium{
enum class AudioEventKind:std::uint8_t{Impact,Weapon,Machine,Environment,Ui,Alert,Dialogue,MusicStinger};
struct AudioEvent{std::uint64_t eventId{},sourceStableId{};AudioEventKind kind{};float x{},y{},z{},gain{1},pitch{1};std::uint8_t priority{};std::uint32_t soundId{};};
class AudioEventQueue{public:void emit(AudioEvent event);std::vector<AudioEvent>drain(std::size_t maxEvents);void clear(){events_.clear();}private:std::vector<AudioEvent>events_;};
}
