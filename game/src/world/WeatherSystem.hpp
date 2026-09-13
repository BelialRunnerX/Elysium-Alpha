// Intended function: deterministic planetary weather state machine exposing gameplay hazards, visibility and renewable-energy modifiers.
#pragma once
#include <cstdint>
namespace elysium{
enum class WeatherKind:std::uint8_t{Clear,Cloudy,Rain,HeavyRain,Thunderstorm,Fog,Dust,Haboob,Snow,Blizzard,Hail,HeatWave,ColdSnap,AcidRain,ElectricalStorm,RadiationStorm,SporeBloom,AshFall,CryoMist,RiftAurora};
struct WeatherState{WeatherKind kind{WeatherKind::Clear};std::uint64_t startTick{},endTick{},transitionSeed{};float intensity{};};
struct WeatherEffects{float visibility{1},temperatureDelta{},radiation{},corrosion{},wind{},precipitation{},solarMultiplier{1},sensorNoise{};};
WeatherState nextWeather(std::uint64_t planetSeed,std::uint64_t cellKey,std::uint64_t tick,float humidity,float temperature,float anomaly);
WeatherEffects weatherEffects(const WeatherState&state);
}
