#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
namespace elysium::terms {
using TermId=std::uint64_t;struct Term{TermId id{};std::string preferred,definition,owner,symbol,serializedKey;std::vector<std::string>aliases,legacyAliases;bool deprecated{};};struct Issue{TermId id{};std::string code;};class Manifest{public:explicit Manifest(std::uint32_t version=1):version_(version){}bool add(Term);std::optional<Term>find(TermId)const;std::vector<Issue>audit()const;std::string glossary()const;std::uint32_t version()const{return version_;}static Manifest seeded();private:std::uint32_t version_;std::map<TermId,Term>terms_;};
}
