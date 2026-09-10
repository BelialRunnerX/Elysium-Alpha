#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace elysium::construction {
using ContentId=std::uint64_t;
enum class ShapeFamily:std::uint8_t{Wall,Floor,Roof,Beam,Column,Stair,Ladder,Ramp,Opening,Window,Railing,Hardpoint,Decor,Pipe,Duct,Cable,Conveyor};
enum class AnchorKind:std::uint8_t{Cell,Face,Edge,Corner}; enum class PieceClass:std::uint8_t{Passive,Interactive};
struct PhysicalMask{bool collision{},seal{},support{},navigation{},clearance{},microRefinable{};double permeability{};};
struct CostLine{ContentId content{},substitutionGroup{};std::uint64_t amount{};};
struct PrefabDef{ContentId id{};ShapeFamily family{};PieceClass classification{};std::uint8_t allowedRotationMask{0x0F};bool mirrorAllowed{};AnchorKind anchor{AnchorKind::Cell};std::int32_t sizeX{1},sizeY{1},sizeZ{1};PhysicalMask mask{};std::vector<CostLine>costs;std::vector<ContentId>functionalCapabilities;};
struct SnapRequest{ContentId prefabId{};std::int64_t cellX{},cellY{},cellZ{};std::uint8_t quarterTurn{};bool mirror{};AnchorKind anchor{AnchorKind::Cell};};
struct SnapResult{bool legal{};std::string blocker;SnapRequest normalized{};PhysicalMask mask{};std::vector<CostLine>costs;};
class PrefabRegistry{public:bool add(PrefabDef);const PrefabDef*find(ContentId)const;std::vector<std::string>validate()const;SnapResult preview(SnapRequest)const;private:std::map<ContentId,PrefabDef>defs_;};
}
