// Intended function: multi-file save generation manifest for crash-safe publication of world deltas, objects, players and strategic state.
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
namespace elysium{
struct SaveFileRecord{std::string logicalName,path;std::uint64_t generation{},checksum{},bytes{};bool required{true};};
struct SaveGenerationManifest{std::uint32_t schema{1};std::uint64_t worldId{},activeGeneration{},previousGeneration{};std::vector<SaveFileRecord> files;};
struct SavePublicationPlan{SaveGenerationManifest next;std::vector<std::pair<std::string,std::string>> atomicRenames;std::vector<std::string> pruneAfterPublish;};
bool validateSaveManifest(const SaveGenerationManifest& manifest,std::string* error=nullptr);
std::string serializeSaveManifest(const SaveGenerationManifest& manifest);
std::optional<SaveGenerationManifest> parseSaveManifest(const std::string& text,std::string* error=nullptr);
SavePublicationPlan planSavePublication(const SaveGenerationManifest& current,std::vector<SaveFileRecord> staged);
}
