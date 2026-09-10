#pragma once
#include <cstdint>
#include <string_view>
#include <vector>
namespace elysium::formula {
enum class ConstantClass:std::uint8_t{FormatIdentity,LockedRule,Tuning};
struct ConstantMeta{std::uint64_t id{};std::string_view name;ConstantClass classification{};double value{};std::uint32_t version{1};};
struct FormulaTrace{std::uint64_t formulaId{};std::uint32_t version{1};double inputA{},inputB{},inputC{},output{};std::string_view explanation;};
inline constexpr std::uint32_t FormulaVersion=1;
inline constexpr std::uint32_t ChunkEdgeMacro=32;
inline constexpr std::uint32_t MicroResolution=16;
inline constexpr double MacroMeters=1.0;
inline constexpr std::uint32_t AddressFormatVersion=2; // 64-bit sparse system/world addressing; no fixed system count.

double combineShare(double a,double b) noexcept;
double curvedStat(double value,double halfway,double ceiling) noexcept;
std::uint64_t xpNext(std::uint64_t level) noexcept;
double tierScale(std::uint32_t tier,double growth=1.25) noexcept;
double statWeight(std::uint32_t slot,std::uint32_t count) noexcept;
double oreDepthWeight(double normalizedDepth,double preferredDepth,double spread) noexcept;
std::uint64_t warpCellCost(double distance,double efficiency=1.0) noexcept;
double claimFloorPressure(double claimStrength,double hostilePressure) noexcept;
double dispatchChance(double standing) noexcept;
double enemyHealthScale(double level) noexcept;
double enemyDamageScale(double level) noexcept;
double enemyArmorScale(double level) noexcept;
double bossHealthScale(double level) noexcept;
FormulaTrace trace(std::uint64_t formulaId,double a,double b,double c,double output,std::string_view explanation) noexcept;
std::vector<ConstantMeta> canonicalConstants();
}
