#pragma once
#include "core/ReasonStack.hpp"
#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>
namespace elysium::automation {
using StableId=std::uint64_t;using SignalId=std::uint64_t;using ActionId=std::uint64_t;
enum class Comparison:std::uint8_t{Always,Equal,NotEqual,Less,LessEqual,Greater,GreaterEqual,RisingEdge,FallingEdge};
struct SignalValue{SignalId id{};double numeric{};bool boolean{};std::uint64_t revision{};};
struct TcaRule{StableId ruleId{};SignalId trigger{};Comparison comparison{Comparison::Always};double threshold{};ActionId action{};StableId target{};std::uint16_t priority{100};bool enabled{true};};
struct ActionRequest{StableId ruleId{};ActionId action{};StableId target{};double sourceValue{};};
class TcaRuntime{public:[[nodiscard]]std::vector<ActionRequest>evaluate(std::span<const TcaRule>rules,std::span<const SignalValue>signals);void reset();private:std::unordered_map<StableId,double>previous_;};
} // namespace elysium::automation
