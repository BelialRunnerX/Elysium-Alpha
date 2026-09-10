#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::fortress {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class AlertPriority:std::uint8_t{Advisory,High,Critical};struct AlertKey{ContentId category{};StableId source{},subject{};friend bool operator==(const AlertKey&,const AlertKey&)=default;};struct Alert{AlertKey key;AlertPriority priority{};std::uint64_t first{},last{},count{};bool acknowledged{},pinned{},muted{},active{true};ContentId reason{};};struct KeyHash{std::size_t operator()(const AlertKey&k)const{return std::size_t(k.category^(k.source<<1)^(k.subject<<7));}};class AlertRuntime{public:void signal(AlertKey,AlertPriority,std::uint64_t,ContentId);bool clear(AlertKey,std::uint64_t);bool acknowledge(AlertKey);std::vector<Alert>active()const;private:std::unordered_map<AlertKey,Alert,KeyHash>rows_;};}