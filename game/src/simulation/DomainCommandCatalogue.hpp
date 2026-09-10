#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::simulation {using StableId=std::uint64_t;using ContentId=std::uint64_t;struct CommandSchema{ContentId type{},owner{};std::uint32_t version{1};};struct DomainCommand{StableId id{},issuer{},target{};ContentId type{};std::uint32_t version{1};std::uint64_t expectedRevision{},address{},tieBreak{};std::int32_t priority{};std::vector<std::uint8_t>payload;};struct CommandResult{bool committed{false};std::string reason;std::uint64_t newRevision{};};class DomainCommandCatalogue{public:bool addSchema(CommandSchema,std::string&);CommandResult validate(const DomainCommand&,StableId authorizedIssuer,std::uint64_t currentRevision)const;ContentId ownerOf(ContentId)const;private:std::unordered_map<ContentId,CommandSchema>schemas_;};}