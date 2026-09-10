// Intended function: Provide stable dialogue nodes, speaker roles, conditions, choices, consequences, and localization keys.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace elysium::content {
struct DialogueNode {
    std::uint64_t nodeId{};
    std::uint64_t speakerRole{};
    double conditionHash{};
    std::uint64_t choiceHash{};
    std::uint64_t consequenceHash{};
    std::uint64_t textKey{};
};
class DialogueNodeRegistry {
public:
    bool publish(DialogueNode record);
    bool remove(std::uint64_t key);
    [[nodiscard]] const DialogueNode* lookup(std::uint64_t key) const;
    [[nodiscard]] const std::vector<DialogueNode>& records() const noexcept { return records_; }
private:
    static std::uint64_t key(const DialogueNode& r) noexcept;
    std::vector<DialogueNode> records_;
};
} // namespace elysium::content
