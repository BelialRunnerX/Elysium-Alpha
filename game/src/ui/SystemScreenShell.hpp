#pragma once
#include <cstdint>
#include <map>
#include <vector>
namespace elysium::ui {
enum class ScreenDomain:std::uint8_t{InventoryCrafting,PowerLogistics,Map,Research,ShipLoadout,StandingFile,Automation};
enum class ScreenCommandKind:std::uint8_t{Select,RequestTransfer,RequestCraft,RequestRoute,RequestResearch,RequestModuleChange,RequestAutomationChange};
struct ReasonRow{std::uint64_t code{},sourceStableId{};double value{};};
struct DomainRow{std::uint64_t stableId{},contentId{};double value{},capacity{};std::uint64_t flags{};std::vector<ReasonRow>reasons;};
struct DomainViewModel{ScreenDomain domain{};std::uint64_t sourceRevision{};std::vector<DomainRow>rows;bool chartedOnly{},localNetworkOnly{},readOnlyLedger{};};
struct ScreenCommandIntent{std::uint64_t stableId{},targetStableId{},expectedRevision{};ScreenDomain domain{};ScreenCommandKind kind{};std::uint64_t contentId{},quantity{};};
struct ScreenNavigationState{ScreenDomain active{};std::uint64_t selectedStableId{},scrollAnchor{};};
class SystemScreenShell{public:bool publish(DomainViewModel);[[nodiscard]]const DomainViewModel*view(ScreenDomain)const;bool restoreNavigation(ScreenNavigationState);[[nodiscard]]ScreenNavigationState navigation()const{return nav_;}[[nodiscard]]ScreenCommandIntent command(ScreenCommandIntent)const;private:std::map<ScreenDomain,DomainViewModel>views_;ScreenNavigationState nav_{};};
} // namespace elysium::ui
