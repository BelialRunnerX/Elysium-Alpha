#include "content/MachineSchema.hpp"
#include <algorithm>
#include <cmath>
namespace elysium::content {namespace{double nn(double v){if(std::isnan(v)||v<0)return 0;if(!std::isfinite(v))return 4294967295.0;return std::min(v,4294967295.0);} }
bool MachineSchemaRegistry::addKnownReference(ContentId id){if(frozen_||!id)return false;refs_.insert(id);return true;}
bool MachineSchemaRegistry::add(MachineSchema d){if(frozen_||!d.id||defs_.contains(d.id))return false;d.powerGeneration=nn(d.powerGeneration);d.powerDraw=nn(d.powerDraw);for(auto&p:d.ports)if(p.capacity>0&&p.id==0)return false;defs_[d.id]=std::move(d);return true;}
std::vector<MachineValidation> MachineSchemaRegistry::validate()const{std::vector<MachineValidation>o;for(auto&[id,d]:defs_){if((d.powerDraw>0||d.powerGeneration>0)&&!d.priorityDeclared)o.push_back({id,"powered_without_brownout_priority"});if(d.tickPolicy==TickPolicy::FixedInterval&&d.fixedIntervalTicks==0)o.push_back({id,"fixed_interval_zero"});if(!d.persistenceSchema)o.push_back({id,"missing_persistence_schema"});for(auto&p:d.ports){if(!p.id)o.push_back({id,"invalid_port"});if(p.filter&&!refs_.contains(p.filter))o.push_back({id,"missing_port_filter_reference"});if(!p.input&&!p.output)o.push_back({id,"port_has_no_direction"});}for(auto r:d.processRefs)if(!refs_.contains(r))o.push_back({id,"missing_process_reference"});for(auto&e:d.environment)if(e.id&&!refs_.contains(e.id))o.push_back({id,"missing_environment_reference"});static const std::set<std::string> allowed{"idle","active","fault","damaged","powered"};for(auto&s:d.visualStates)if(!allowed.contains(s))o.push_back({id,"invalid_visual_state"});}return o;}
bool MachineSchemaRegistry::freeze(){if(!validate().empty())return false;frozen_=true;return true;}
const MachineSchema*MachineSchemaRegistry::find(ContentId id)const{auto i=defs_.find(id);return i==defs_.end()?nullptr:&i->second;}
AuthoringFacade::AuthoringFacade(const BlockSchemaRegistry&b,const MachineSchemaRegistry&m):blocks_(b),machines_(m){}
AuthoringQuery AuthoringFacade::queryBlock(ContentId id)const{auto b=blocks_.resolve(id);return b?AuthoringQuery{AuthoringQuery::Kind::Block,id,b->harvestTier,false}:AuthoringQuery{};}
AuthoringQuery AuthoringFacade::queryMachine(ContentId id)const{auto*m=machines_.find(id);return m?AuthoringQuery{AuthoringQuery::Kind::Machine,id,m->tier,true}:AuthoringQuery{};}
}
