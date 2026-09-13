// Intended function: future multiplayer replication envelope using stable IDs and versioned deltas without exposing transient ECS/GPU handles.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace elysium{
enum class ReplicationOp:std::uint8_t{Create,Update,Destroy,Tombstone};
struct ReplicatedField{std::uint32_t fieldId{};std::vector<std::uint8_t> bytes;};
struct EntityReplicationDelta{std::uint64_t stableId{};std::uint32_t archetypeId{},revision{};ReplicationOp op{ReplicationOp::Update};std::vector<ReplicatedField> fields;};
struct ReplicationPacket{std::uint16_t schema{1};std::uint64_t sessionId{},sequence{},ackSequence{},worldGeneration{};std::vector<EntityReplicationDelta>deltas;};
std::vector<std::uint8_t> encodeReplicationPacket(const ReplicationPacket& packet);
std::optional<ReplicationPacket> decodeReplicationPacket(const std::vector<std::uint8_t>& bytes,std::string* error=nullptr);
class ReplicationWindow{public:void noteSent(std::uint64_t seq);void acknowledge(std::uint64_t seq);bool shouldResend(std::uint64_t seq)const;std::uint64_t highestAck()const{return highestAck_;}private:std::vector<std::uint64_t>outstanding_;std::uint64_t highestAck_{};};
}
