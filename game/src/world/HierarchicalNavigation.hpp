// Intended function: sparse long-distance navigation over address-keyed sectors with cancellation, dynamic-obstacle revisions and route reuse.
#pragma once
#include <cstdint>
#include <deque>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace elysium {

enum class NavObstacleClass : std::uint8_t { None, Soft, Solid, Hazard, Restricted, VehicleOnly };
struct NavSectorKey { std::uint64_t address{}; friend bool operator==(const NavSectorKey&,const NavSectorKey&)=default; };
struct NavSectorEdge { NavSectorKey to{}; float cost{1.f}; std::uint32_t capabilityMask{~0u}; };
struct NavRouteRequest { std::uint64_t requestId{}, actorStableId{}; NavSectorKey start{}, goal{}; std::uint32_t capabilityMask{~0u}; std::uint32_t maxExpanded{2048}; };
struct NavRoutePlan { std::uint64_t requestId{}; bool found{}, cancelled{}, fromCache{}; std::uint64_t obstacleRevision{}; std::vector<NavSectorKey> sectors; float totalCost{}; };

class HierarchicalNavigation {
public:
    void setEdges(NavSectorKey from, std::vector<NavSectorEdge> edges);
    void setObstacle(NavSectorKey sector, NavObstacleClass obstacle);
    void clearObstacle(NavSectorKey sector);
    std::uint64_t obstacleRevision() const { return obstacleRevision_; }
    void submit(NavRouteRequest request);
    void cancel(std::uint64_t requestId);
    std::vector<NavRoutePlan> process(std::size_t maxRequests);
    std::optional<NavRoutePlan> cached(NavSectorKey start, NavSectorKey goal, std::uint32_t capabilities) const;

private:
    struct KeyHash { std::size_t operator()(NavSectorKey k) const noexcept { return std::size_t(k.address^(k.address>>33U)); } };
    struct CacheKey { std::uint64_t a{},b{},revision{}; std::uint32_t caps{}; friend bool operator==(const CacheKey&,const CacheKey&)=default; };
    struct CacheHash { std::size_t operator()(const CacheKey& k) const noexcept; };
    NavRoutePlan solve(const NavRouteRequest& request);
    std::unordered_map<NavSectorKey,std::vector<NavSectorEdge>,KeyHash> graph_;
    std::unordered_map<NavSectorKey,NavObstacleClass,KeyHash> obstacles_;
    std::unordered_map<CacheKey,NavRoutePlan,CacheHash> cache_;
    std::deque<NavRouteRequest> pending_;
    std::unordered_set<std::uint64_t> cancelled_;
    std::uint64_t obstacleRevision_{1};
};

} // namespace elysium
