#include "base/PowerPolicy.hpp"
#include <algorithm>
namespace elysium::base {
std::vector<PowerPolicyDecision> PowerPolicy::evaluate(std::span<const PowerPolicyInput> in,bool emergency)const{std::vector<PowerPolicyDecision>out;out.reserve(in.size());for(const auto&n:in)out.push_back({n.nodeId,emergency&&n.emergencyCritical?n.emergencyPriority:n.normalPriority,n.manuallyIsolated||n.faulted,!n.faulted&&!n.manuallyIsolated});std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){if(a.effectivePriority!=b.effectivePriority)return a.effectivePriority<b.effectivePriority;return a.nodeId<b.nodeId;});return out;}
} // namespace elysium::base
