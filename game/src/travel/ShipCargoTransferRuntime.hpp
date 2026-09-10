#pragma once
#include <cstdint>
#include <map>
#include <vector>

namespace elysium::travel {

enum class CargoEndpointKind : std::uint8_t { ShipHold, CargoLoader, HangarPad, BaseLocalNetworkStorage };
struct CargoStack { std::uint64_t itemStableId{}, contentId{}, count{}; };
struct CargoEndpoint {
    std::uint64_t stableId{};
    CargoEndpointKind kind{CargoEndpointKind::ShipHold};
    std::uint64_t ownerStableId{};
    std::uint64_t baseStableId{};
    std::uint64_t capacity{};
    bool available{true};
    bool powered{true};
    std::vector<CargoStack> stacks;
};
enum class CargoTransferState : std::uint8_t { Requested, Reserved, InProgress, Paused, Completed, Cancelled };
enum class CargoTransferFailure : std::uint8_t { None, MissingEndpoint, InvalidRequest, PermissionDenied, DockUnavailable, Brownout, CapacityExceeded, EndpointBusy, BaseBoundaryViolation, MissingCargo, RevisionConflict };
struct CargoTransferTransaction {
    std::uint64_t stableId{};
    std::uint64_t authorStableId{};
    std::uint64_t sourceEndpointId{};
    std::uint64_t destinationEndpointId{};
    std::uint64_t itemStableId{};
    std::uint64_t contentId{};
    std::uint64_t requestedCount{};
    std::uint64_t movedCount{};
    std::uint64_t revision{1};
    CargoTransferState state{CargoTransferState::Requested};
    CargoTransferFailure lastFailure{CargoTransferFailure::None};
};
enum class CargoEventKind : std::uint8_t { Arrival, Departure, TransferStarted, TransferPaused, TransferCompleted, TransferCancelled };
struct CargoTransferEvent { std::uint64_t sequence{}; CargoEventKind kind{}; std::uint64_t transactionId{}, endpointId{}, relatedEndpointId{}, count{}; };
struct CargoTransferSnapshot { std::vector<CargoEndpoint> endpoints; std::vector<CargoTransferTransaction> transactions; std::uint64_t nextEventSequence{1}; };

class ShipCargoTransferRuntime {
public:
    bool publishEndpoint(CargoEndpoint endpoint);
    bool createTransfer(CargoTransferTransaction transaction);
    CargoTransferFailure reserve(std::uint64_t transactionId);
    CargoTransferFailure step(std::uint64_t transactionId, std::uint64_t maximumCount);
    bool cancel(std::uint64_t transactionId);
    bool setEndpointPower(std::uint64_t endpointId,bool powered);
    bool setEndpointAvailability(std::uint64_t endpointId,bool available);
    void emitArrival(std::uint64_t endpointId,std::uint64_t relatedEndpointId);
    void emitDeparture(std::uint64_t endpointId,std::uint64_t relatedEndpointId);
    [[nodiscard]] const CargoEndpoint* endpoint(std::uint64_t id) const;
    [[nodiscard]] const CargoTransferTransaction* transaction(std::uint64_t id) const;
    [[nodiscard]] const std::vector<CargoTransferEvent>& events() const noexcept{return events_;}
    [[nodiscard]] CargoTransferSnapshot snapshot() const;
    bool restore(const CargoTransferSnapshot& snapshot);
private:
    void event(CargoEventKind kind,std::uint64_t transactionId,std::uint64_t endpointId,std::uint64_t related,std::uint64_t count);
    bool endpointLocked(std::uint64_t endpointId,std::uint64_t excludingTransaction=0) const;
    std::map<std::uint64_t,CargoEndpoint> endpoints_;
    std::map<std::uint64_t,CargoTransferTransaction> transactions_;
    std::vector<CargoTransferEvent> events_;
    std::uint64_t nextEventSequence_{1};
};
} // namespace elysium::travel
