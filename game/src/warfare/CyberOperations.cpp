#include "warfare/CyberOperations.hpp"
#include <algorithm>
namespace elysium {
// Intended function: Track network intrusion, malware incidents, defensive posture, compromised systems, and recovery intents.
bool CyberOperationsStore::apply(const CyberOperationsOp& o) { if(!o.id) return false; auto& d=data_[o.id]; d.revision=revision_++; d.id=o.id; d.owner=o.owner; d.ref=o.ref; d.value=o.value; d.flags=o.flags; d.live=true; return true; }
bool CyberOperationsStore::erase(std::uint64_t id) { return data_.erase(id)!=0; }
const CyberOperationsData* CyberOperationsStore::find(std::uint64_t id) const { auto it=data_.find(id); return it==data_.end()?nullptr:&it->second; }
std::vector<CyberOperationsData> CyberOperationsStore::snapshot() const { std::vector<CyberOperationsData> v; v.reserve(data_.size()); for(auto& [id,d]:data_) if(d.live) v.push_back(d); std::sort(v.begin(),v.end(),[](const auto&a,const auto&b){return a.id<b.id;}); return v; }
void CyberOperationsStore::clear() { data_.clear(); revision_=1; }
}
