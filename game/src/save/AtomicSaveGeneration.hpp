#pragma once
#include "save/SpatialDeltaCodec.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <vector>
namespace elysium::save {
struct SaveGenerationMeta{std::uint64_t generationId{},previousGenerationId{},generatorFingerprint{};std::uint32_t formatVersion{1},spatialSchemaVersion{1};std::uint64_t checksum{};};
struct SaveGeneration{SaveGenerationMeta meta;std::vector<std::uint8_t>payload;};
class IAtomicSaveStore{public:virtual~IAtomicSaveStore()=default;virtual bool writeTemp(std::uint64_t,const std::vector<std::uint8_t>&)=0;virtual bool flushTemp(std::uint64_t)=0;virtual bool publishTemp(std::uint64_t)=0;virtual bool readPublished(std::uint64_t,std::vector<std::uint8_t>&)const=0;virtual bool setManifest(std::uint64_t)=0;virtual std::uint64_t manifest()const=0;};
enum class SavePublishFailure:std::uint8_t{None,Invalid,WriteFailed,FlushFailed,PublishFailed,ManifestFailed,ChecksumMismatch,NoGoodGeneration};
class AtomicSaveGeneration{public:static std::uint64_t checksum(const std::vector<std::uint8_t>&);[[nodiscard]]std::vector<std::uint8_t>pack(const SaveGeneration&)const;[[nodiscard]]SavePublishFailure unpack(const std::vector<std::uint8_t>&,SaveGeneration&)const;SavePublishFailure publish(IAtomicSaveStore&,SaveGeneration);SavePublishFailure recover(const IAtomicSaveStore&,SaveGeneration&)const;[[nodiscard]]SpatialDeltaBatch compact(const SpatialDeltaBatch&)const;};
} // namespace elysium::save
