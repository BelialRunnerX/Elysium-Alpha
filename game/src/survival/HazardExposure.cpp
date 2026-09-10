#include "survival/HazardExposure.hpp"
#include "core/Saturating.hpp"
#include <algorithm>
namespace elysium::survival {
HazardExposureResult HazardExposureAggregator::evaluate(std::span<const HazardSourceSample>s,std::span<const ResistanceSource>rs)const{HazardExposureResult o{};std::vector<HazardSourceSample>src(s.begin(),s.end());std::sort(src.begin(),src.end(),[](auto&a,auto&b){if(a.hazard!=b.hazard)return a.hazard<b.hazard;if(a.providerId!=b.providerId)return a.providerId<b.providerId;return a.sourceId<b.sourceId;});for(const auto&r:rs){auto i=static_cast<std::size_t>(r.hazard);if(i>=kSurvivalHazardCount)continue;double q=safe::finiteClamp(r.share01,0.0,0.999999999);o.resistance[i]=1.0-(1.0-o.resistance[i])*(1.0-q);}for(const auto&x:src){auto i=static_cast<std::size_t>(x.hazard);if(i>=kSurvivalHazardCount)continue;double raw=x.blockedByLocalEngineering?0.0:safe::nonNegative(x.intensityPerSecond);double eff=safe::nonNegative(raw*(1.0-o.resistance[i]));o.raw[i]=safe::nonNegative(o.raw[i]+raw);o.effective[i]=safe::nonNegative(o.effective[i]+eff);o.contributions.push_back({x.providerId,x.sourceId,raw,eff});}return o;}
} // namespace elysium::survival
