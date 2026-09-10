#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
namespace elysium::construction {
using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class NetworkOwner:std::uint8_t{Power,Fluid,Atmosphere,Logistics,Automation,Defense};
struct EndpointDecl{std::uint32_t localSlot{};NetworkOwner owner{};ContentId portType{};bool input{},output{};bool decorative{};};
struct EndpointState{StableId endpointId{},objectId{};EndpointDecl decl{};bool enabled{true};};
struct RegistrationEvent{enum class Kind:std::uint8_t{Register,Unregister,WakeOwner};Kind kind{};NetworkOwner owner{};StableId endpointId{},objectId{};};
class PrefabNetworkLifecycle{public:bool place(StableId objectId,const std::vector<EndpointDecl>&,std::vector<RegistrationEvent>&);bool disable(StableId,std::vector<RegistrationEvent>&);bool destroy(StableId,std::vector<RegistrationEvent>&);std::vector<EndpointState>snapshot()const;std::vector<std::string>preview(const std::vector<EndpointDecl>&)const;private:std::map<StableId,std::vector<EndpointState>>objects_;static StableId endpointId(StableId,std::uint32_t,NetworkOwner);};
}
