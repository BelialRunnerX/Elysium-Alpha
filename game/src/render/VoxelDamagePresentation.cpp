// Intended function: generate deterministic cosmetic damage particles without feeding any presentation state back into world simulation.
#include "render/VoxelDamagePresentation.hpp"
#include "core/Determinism.hpp"
#include <algorithm>
namespace elysium{VoxelDamagePresentation buildVoxelDamagePresentation(const VoxelDamageVisualEvent&e,std::uint32_t budget){VoxelDamagePresentation o;auto n=std::min<std::uint32_t>(budget,std::uint32_t(4+std::clamp(e.severity,0.f,1.f)*28));for(std::uint32_t i=0;i<n;++i){auto h=mix64(e.eventId^i);auto f=[&](int sh){return float((h>>sh)&255)/255.f;};o.debris.push_back({e.x+(f(0)-.5f)*.4f,e.y+(f(8)-.5f)*.4f,e.z+(f(16)-.5f)*.4f,(f(24)-.5f)*5.f,f(32)*5.f,(f(40)-.5f)*5.f,.03f+f(48)*.18f,.3f+f(56)*1.7f,e.materialId});}o.scorches.push_back({e.x,e.y,e.z,.2f+e.severity*1.2f,std::clamp(e.severity,0.f,1.f),mix64(e.eventId^0x53434F524348ULL)});return o;}}
