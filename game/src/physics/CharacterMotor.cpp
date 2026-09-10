// Intended function: Track desired movement, grounded state, jump/fly mode, velocity, collision response, slope limits, and fall-damage inputs.
#include "CharacterMotor.hpp"
namespace elysium::physics {
std::uint64_t CharacterMotorStateIndex::keyOf(const CharacterMotorState& v) noexcept { return static_cast<std::uint64_t>(v.actorId); }
bool CharacterMotorStateIndex::upsert(CharacterMotorState v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterMotorState& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool CharacterMotorStateIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterMotorState& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const CharacterMotorState* CharacterMotorStateIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const CharacterMotorState& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
