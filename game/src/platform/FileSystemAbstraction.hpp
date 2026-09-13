// Intended function: Represent platform-independent save/config/cache/mod/log paths and atomic file operation requests.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::platform {
struct FileOperation {
    std::uint64_t operationId{};
    std::uint64_t kind{};
    std::uint64_t pathHash{};
    std::uint64_t payloadHash{};
    std::uint64_t flags{};
    std::uint64_t sequence{};
};
class FileOperationCollection {
public:
 bool store(FileOperation value); bool erase(std::uint64_t id); [[nodiscard]] const FileOperation* find(std::uint64_t id) const; [[nodiscard]] const std::vector<FileOperation>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const FileOperation& v) noexcept; std::vector<FileOperation> rows_;
};
}
