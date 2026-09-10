// Intended function: powered stable portal endpoint registry and transit request planner for local/planetary transport without storing raw runtime handles.
#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>
namespace elysium{
struct PortalEndpoint{std::uint64_t portalId{},address{},networkId{};float powerRequired{},stability{1};bool enabled{true},sealed{};};
struct PortalTransitRequest{std::uint64_t requestId{},actorId{},fromPortal{},toPortal{};float mass{};};
struct PortalTransitPlan{bool allowed{};std::uint64_t requestId{},actorId{},fromAddress{},toAddress{};float energyCost{};std::uint32_t failureCode{};};
class PortalNetwork{public:void upsert(PortalEndpoint p);PortalTransitPlan plan(const PortalTransitRequest&r,float availablePower)const;std::vector<PortalEndpoint>network(std::uint64_t networkId)const;private:std::unordered_map<std::uint64_t,PortalEndpoint>portals_;};
}
