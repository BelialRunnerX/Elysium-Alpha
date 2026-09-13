#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::simulation {using StableId=std::uint64_t;using ContentId=std::uint64_t;enum class Retention:std::uint8_t{Transient,Durable,ChronicleCandidate};struct EventSchema{ContentId type{};std::uint32_t version{1};ContentId producerOwner{};std::vector<ContentId>consumerOwners;Retention retention{Retention::Transient};};struct DomainEvent{StableId id{};ContentId type{};std::uint32_t version{1};StableId source{},subject{};std::uint64_t address{},epoch{},orderKey{};StableId cause{},reference{};std::vector<std::uint8_t>payload;};class DomainEventCatalogue{public:bool addSchema(EventSchema,std::string&);bool validate(const DomainEvent&,std::string&)const;std::vector<DomainEvent>order(std::vector<DomainEvent>)const;private:std::unordered_map<ContentId,EventSchema>schemas_;};}