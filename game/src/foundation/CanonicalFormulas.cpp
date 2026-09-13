#include "foundation/CanonicalFormulas.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
namespace elysium::formula { namespace {double nn(double v){if(std::isnan(v)||v<0)return 0;if(!std::isfinite(v))return 4294967295.0;return std::min(v,4294967295.0);} }
double combineShare(double a,double b)noexcept{a=std::clamp(nn(a),0.0,0.999);b=std::clamp(nn(b),0.0,0.999);return std::clamp(1.0-(1.0-a)*(1.0-b),0.0,0.999999);}
double curvedStat(double v,double h,double c)noexcept{v=nn(v);h=std::max(1e-9,nn(h));c=nn(c);return std::min(nn((v/(v+h))*c),c);}
std::uint64_t xpNext(std::uint64_t level)noexcept{constexpr auto M=std::numeric_limits<std::uint64_t>::max();if(level>(M-60)/40)return M;return 60+40*level;}
double tierScale(std::uint32_t tier,double growth)noexcept{growth=std::max(1.0,nn(growth));double out=1.0;for(std::uint32_t i=0;i<tier;++i){if(out>4294967295.0/growth)return 4294967295.0;out*=growth;}return nn(out);}
double statWeight(std::uint32_t slot,std::uint32_t count)noexcept{if(count==0||slot>=count)return 0;const double center=(double(count)-1.0)*0.5;const double d=std::abs(double(slot)-center);return 1.0/(1.0+d);}
double oreDepthWeight(double d,double p,double s)noexcept{d=std::clamp(nn(d),0.0,1.0);p=std::clamp(nn(p),0.0,1.0);s=std::max(1e-4,std::min(nn(s),1.0));const double z=(d-p)/s;return std::min(nn(std::exp(-0.5*z*z)),1.0);}
std::uint64_t warpCellCost(double distance,double efficiency)noexcept{distance=nn(distance);efficiency=std::max(1e-6,nn(efficiency));double v=std::ceil(distance/efficiency);if(v>=double(std::numeric_limits<std::uint64_t>::max()))return std::numeric_limits<std::uint64_t>::max();return static_cast<std::uint64_t>(v);}
double claimFloorPressure(double claim,double hostile)noexcept{return nn(std::min(nn(claim),nn(hostile)));}
double dispatchChance(double standing)noexcept{standing=nn(standing);if(standing<25.0)return 0.0;return std::clamp((standing-25.0)/200.0,0.0,0.40);}
double enemyHealthScale(double level)noexcept{level=std::max(1.0,nn(level));return nn(1.0+0.30*std::sqrt(level-1.0));}
double enemyDamageScale(double level)noexcept{level=std::max(1.0,nn(level));return nn(1.0+0.045*(level-1.0));}
double enemyArmorScale(double level)noexcept{level=std::max(1.0,nn(level));return nn(1.0+0.20*std::sqrt(level-1.0));}
double bossHealthScale(double level)noexcept{level=std::max(1.0,nn(level));return nn(2.0+0.6*std::sqrt(level-1.0));}
FormulaTrace trace(std::uint64_t id,double a,double b,double c,double o,std::string_view e)noexcept{return{id,FormulaVersion,nn(a),nn(b),nn(c),nn(o),e};}
std::vector<ConstantMeta> canonicalConstants(){return{{1,"chunk_edge_macro",ConstantClass::FormatIdentity,double(ChunkEdgeMacro),AddressFormatVersion},{2,"micro_resolution",ConstantClass::FormatIdentity,double(MicroResolution),AddressFormatVersion},{3,"macro_meters",ConstantClass::FormatIdentity,MacroMeters,AddressFormatVersion},{4,"dispatch_notice_threshold",ConstantClass::LockedRule,25.0,FormulaVersion},{5,"tier_growth",ConstantClass::Tuning,1.25,FormulaVersion}};}
}
