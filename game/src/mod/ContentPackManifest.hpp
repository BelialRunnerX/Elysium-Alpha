#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
namespace elysium::mod {
using ContentId=std::uint64_t; using PackId=std::uint64_t;
struct SemVer{std::uint32_t major{},minor{},patch{}; friend bool operator==(const SemVer&,const SemVer&)=default;};
struct Dependency{PackId id{};std::uint32_t minSchema{1};};
struct Remap{ContentId from{},to{};};
struct PackManifest{PackId id{};std::string nameSpace;SemVer version{};std::vector<Dependency> dependencies;std::set<std::uint32_t> schemas;std::vector<std::string> extensionSurfaces;std::vector<Remap> remaps;};
struct Diagnostic{PackId pack{};std::string message;};
class ContentPackResolver{public:bool add(PackManifest,std::string&);std::vector<PackId> loadOrder(std::vector<Diagnostic>&)const;std::optional<ContentId> remap(PackId,ContentId)const;bool surfaceAllowed(PackId,const std::string&)const;private:std::unordered_map<PackId,PackManifest> packs_;};
}