#include "faction/PoliticalAttention.hpp"
#include <algorithm>
#include <cmath>
#include <tuple>
namespace elysium::faction {
namespace {
double clamp(double x){return std::clamp(x,0.0,100.0);}
bool unit(double x){return std::isfinite(x)&&x>=0&&x<=100;}
double roll(std::uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return double((x^(x>>31))>>11)*0x1.0p-53;}
bool natural(AttentionKind kind){return kind==AttentionKind::RichOre||kind==AttentionKind::OrdinaryOre||kind==AttentionKind::Extractor;}
double total(const SystemAttention& s){return clamp(s.floor+s.pressure);}
}
bool PoliticalAttention::publish(AttentionRule r){
    if(r.kind>=AttentionKind::Count||!std::isfinite(r.favor)||!std::isfinite(r.suspicion)||
       std::abs(r.favor)>100||std::abs(r.suspicion)>100||!std::isfinite(r.chance)||r.chance<0||r.chance>1)return false;
    return rules_.emplace(r.kind,r).second;
}
std::vector<AttentionChange> PoliticalAttention::applyBatch(std::vector<AttentionEvent> events){
    std::sort(events.begin(),events.end(),[](auto& a,auto& b){return std::tie(a.tick,a.id)<std::tie(b.tick,b.id);});
    std::map<std::uint64_t,unsigned> counts;for(auto& e:events)++counts[e.id];
    std::vector<AttentionChange> changes;
    for(auto& e:events){
        AttentionChange c;c.event=e;
        if(!e.id||!e.player||!e.file||!e.system||!e.source||e.kind>=AttentionKind::Count||
           !unit(e.passiveScale)||!unit(e.presenceScale)){c.reason=AttentionReason::Invalid;changes.push_back(c);continue;}
        if(consumed_.count(e.id)||counts[e.id]!=1){c.reason=AttentionReason::Duplicate;changes.push_back(c);continue;}
        auto rule=rules_.find(e.kind);if(rule==rules_.end()){c.reason=AttentionReason::MissingRule;changes.push_back(c);continue;}
        auto& f=files_[{e.player,e.file}];auto& s=f.systems[e.system];s.system=e.system;
        c.favorBefore=c.favorAfter=f.favor;c.suspicionBefore=c.suspicionAfter=total(s);
        c.activeBefore=c.activeAfter=c.suspicionBefore>=25;
        consumed_.insert(e.id);
        if((natural(e.kind)||rule->second.naturalSourceRequired)&&(!e.naturalSource||e.playerPlaced||e.selfGenerated))
            c.reason=AttentionReason::ProvenanceDenied;
        else if(roll(e.id^e.source)>=rule->second.chance)c.reason=AttentionReason::ChanceMiss;
        else{
            const double scale=e.passiveScale*e.presenceScale;
            f.favor=clamp(f.favor+rule->second.favor*scale);
            s.pressure=std::clamp(s.pressure+rule->second.suspicion*scale,0.0,100.0-s.floor);
            c.favorAfter=f.favor;c.suspicionAfter=total(s);c.activeAfter=c.suspicionAfter>=25;c.reason=AttentionReason::Applied;
        }
        changes.push_back(c);
    }
    return changes;
}
bool PoliticalAttention::setClaimFloor(std::uint64_t player,std::uint64_t file,std::uint64_t system,double floor){
    if(!player||!file||!system||!unit(floor))return false;
    auto& s=files_[{player,file}].systems[system];s.system=system;s.floor=floor;s.pressure=std::min(s.pressure,100-floor);return true;
}
bool PoliticalAttention::decay(double seconds,double pressureRate,double favorRate){
    if(!std::isfinite(seconds)||seconds<0||!unit(pressureRate)||!unit(favorRate))return false;
    for(auto& [key,f]:files_){f.favor=clamp(f.favor-seconds*favorRate);for(auto& [id,s]:f.systems)s.pressure=clamp(s.pressure-seconds*pressureRate);}return true;
}
double PoliticalAttention::favor(std::uint64_t player,std::uint64_t file)const{
    auto f=files_.find({player,file});return f==files_.end()?0:f->second.favor;
}
double PoliticalAttention::suspicion(std::uint64_t player,std::uint64_t file,std::uint64_t system)const{
    auto f=files_.find({player,file});if(f==files_.end())return 0;auto s=f->second.systems.find(system);return s==f->second.systems.end()?0:total(s->second);
}
PoliticalSnapshot PoliticalAttention::snapshot()const{
    PoliticalSnapshot out;for(auto& [key,f]:files_){PoliticalFile p{key.first,key.second,f.favor,{}};for(auto& [id,s]:f.systems)p.systems.push_back(s);out.files.push_back(std::move(p));}
    out.consumed.assign(consumed_.begin(),consumed_.end());return out;
}
bool PoliticalAttention::restore(const PoliticalSnapshot& records){
    std::map<Key,FileState> next;std::set<std::uint64_t> ids;
    for(auto& p:records.files){
        if(!p.player||!p.file||!unit(p.favor)||next.count({p.player,p.file}))return false;FileState f;f.favor=p.favor;
        for(auto& s:p.systems)if(!s.system||!unit(s.floor)||!unit(s.pressure)||s.floor+s.pressure>100||!f.systems.emplace(s.system,s).second)return false;
        next.emplace(Key{p.player,p.file},std::move(f));
    }
    for(auto id:records.consumed)if(!id||!ids.insert(id).second)return false;
    files_.swap(next);consumed_.swap(ids);return true;
}
}
