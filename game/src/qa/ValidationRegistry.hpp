#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
namespace elysium::qa {
enum class EvidenceClass:std::uint8_t{Stub,Headless,FakeRenderer,NativeRuntime,MeasuredHardware};
struct TestFamily{std::string name;std::string ownerPackage;EvidenceClass evidence{EvidenceClass::Stub};std::set<std::string> tags;};
struct Regression{std::string name;std::string ownerPackage;std::string family;};
struct AuditIssue{std::string code;std::string subject;};
class ValidationRegistry{public:bool addFamily(TestFamily);bool addRegression(Regression);std::vector<TestFamily> select(const std::set<std::string>& tags)const;std::vector<AuditIssue> audit()const;bool hasRegression(const std::string&)const;private:std::map<std::string,TestFamily> families_;std::map<std::string,Regression> regressions_;};
std::vector<std::string> requiredForeverRegressions();
}
