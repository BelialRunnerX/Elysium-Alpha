// Intended function: Record deterministic input/output hashes, named seed streams, command ordering, and replay checkpoints for debugging later.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::tools {
struct DeterminismCheckpoint {
    std::uint64_t checkpointId{};
    std::uint64_t tick{};
    std::uint64_t inputHash{};
    std::uint64_t outputHash{};
    std::uint64_t commandHash{};
    std::uint64_t workerCount{};
};
class DeterminismCheckpointRegistry {
public:
    bool publish(DeterminismCheckpoint record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const DeterminismCheckpoint* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<DeterminismCheckpoint>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const DeterminismCheckpoint& r) noexcept;
    std::vector<DeterminismCheckpoint> records_;
};
} // namespace elysium::tools
