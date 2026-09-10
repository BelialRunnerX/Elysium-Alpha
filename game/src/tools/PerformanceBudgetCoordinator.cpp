#include "tools/PerformanceBudgetCoordinator.hpp"
#include <algorithm>
#include <cmath>
namespace elysium::perf { namespace { double finite(double v){ if(std::isnan(v)||v<0)return 0; if(!std::isfinite(v))return 4294967295.0; return std::min(v,4294967295.0);} }
bool Coordinator::setBudget(Budget b){ b.target=finite(b.target); b.recoverBelow=finite(b.recoverBelow); if(b.target<=0)return false; if(b.recoverBelow<=0||b.recoverBelow>b.target)b.recoverBelow=b.target*0.85; budgets_[b.metric]=b; return true; }
bool Coordinator::publish(Sample s){ s.value=finite(s.value); auto it=samples_.find(s.metric); if(it!=samples_.end()&&s.tick<it->second.tick)return false; samples_[s.metric]=s; return true; }
Snapshot Coordinator::evaluate(){ bool over=false,recovered=true; for(auto&[m,b]:budgets_){auto it=samples_.find(m);if(it==samples_.end())continue; over|=it->second.value>b.target; recovered&=it->second.value<=b.recoverBelow;} if(over){healthyPasses_=0; if(stage_<Degradation::LowerAnimationRate) stage_=static_cast<Degradation>(static_cast<unsigned>(stage_)+1);} else if(recovered){if(++healthyPasses_>=3&&stage_!=Degradation::None){stage_=static_cast<Degradation>(static_cast<unsigned>(stage_)-1);healthyPasses_=0;}} return {samples_,stage_,over}; }
}
