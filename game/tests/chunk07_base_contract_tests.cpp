#include "automation/AutomationAuthoring.hpp"
#include "automation/TcaRuntime.hpp"
#include "base/BaseDefenseRuntime.hpp"
#include "base/HabitatUtilityRuntime.hpp"
#include "base/PowerPolicy.hpp"
#include "base/ScalarPowerNetwork.hpp"
#include "logistics/BaseLogisticsRuntime.hpp"
#include "world/RoomRecognition.hpp"
#include <array>
#include <cassert>
void chunk07_base_contract_tests_unrun(){using namespace elysium;
 world::RoomFunctionRegistry rr;world::RoomFunctionDefinition rd{};rd.functionId=1;rd.requiresSealed=true;assert(rr.publish(rd));world::BoundedVolumeSnapshot vs{};vs.volumeId=2;vs.terminatedBounded=true;assert(rr.recognize(1,vs).recognized);
 base::ScalarPowerNetwork pw;std::array<base::PowerNodeSnapshot,2>nodes{{{1,1,1,10,0,0,0,0,0,1,false,true},{2,1,1,0,5,0,0,0,0,2,false,true}}};assert(pw.allocate(nodes,1).served>=5);
 logistics::BaseLogisticsRuntime l;assert(l.upsertStorage({1,7,1,10,{{9,5}},true}));assert(l.upsertStorage({2,7,1,10,{},true}));auto tp=l.plan({1,7,9,3,1,2});assert(tp.accepted&&tp.count==3);assert(l.commit({1,7,9,3,1,2},tp));
 automation::TcaRuntime t;std::array<automation::TcaRule,1>rules{{{1,2,automation::Comparison::Greater,5,3,4,1,true}}};std::array<automation::SignalValue,1>s{{{2,6,true,1}}};assert(t.evaluate(rules,s).size()==1);
}
