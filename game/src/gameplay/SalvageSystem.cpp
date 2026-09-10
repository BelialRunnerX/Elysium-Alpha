// Intended function: conserve recoverable matter while varying intact component recovery deterministically with source condition and salvaging capability.
#include "gameplay/SalvageSystem.hpp"
#include "core/Determinism.hpp"
#include <algorithm>
namespace elysium{std::vector<SalvageYield>rollSalvage(std::uint64_t w,const SalvageInput&i){std::vector<SalvageYield>o;auto s=mix64(w^i.sourceStableId^mix64(i.eventOrdinal));float efficiency=std::clamp(.35f+i.skill*.3f+i.toolQuality*.2f+i.condition*.25f,.1f,1.f);std::uint32_t bulk=std::max(1u,std::uint32_t(4.f*efficiency));o.push_back({i.materialId,bulk,std::clamp(i.condition*.8f+.2f,0.f,1.f),mix64(s^1)});float r=float((mix64(s^2)>>40)&0xffffff)/float(0xffffff);if(r<efficiency*.55f)o.push_back({7000u+i.sourceType%512u,1,std::clamp(i.condition*.7f+i.toolQuality*.3f,0.f,1.f),mix64(s^2)});if(r<efficiency*.18f)o.push_back({7600u+i.sourceType%128u,1,std::clamp(i.condition,.1f,1.f),mix64(s^3)});return o;}}
