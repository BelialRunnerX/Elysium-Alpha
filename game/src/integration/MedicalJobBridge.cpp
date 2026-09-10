// Intended function: Translate triage/treatment plans into reservable fortress jobs with patient, bed, tool, medicine, and practitioner dependencies.
#include "MedicalJobBridge.hpp"
namespace elysium::integration {
std::uint64_t MedicalJobIntentIndex::keyOf(const MedicalJobIntent& v) noexcept { return static_cast<std::uint64_t>(v.intentId); }
bool MedicalJobIntentIndex::upsert(MedicalJobIntent v) { auto id=keyOf(v); if(!id)return false; auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MedicalJobIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it!=rows_.end()&&keyOf(*it)==id){*it=v;return true;} rows_.insert(it,v); return true; }
bool MedicalJobIntentIndex::erase(std::uint64_t id) { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MedicalJobIntent& a,std::uint64_t b){return keyOf(a)<b;}); if(it==rows_.end()||keyOf(*it)!=id)return false; rows_.erase(it); return true; }
const MedicalJobIntent* MedicalJobIntentIndex::find(std::uint64_t id) const { auto it=std::lower_bound(rows_.begin(),rows_.end(),id,[](const MedicalJobIntent& a,std::uint64_t b){return keyOf(a)<b;}); return it!=rows_.end()&&keyOf(*it)==id?&*it:nullptr; }
}
