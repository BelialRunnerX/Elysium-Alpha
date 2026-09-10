// Intended function: Project dialogue speaker, lines, choices, conditions, skill/standing checks, consequences, and conversation history.
#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>
namespace elysium::ui {
struct DialogueViewState {
    std::uint64_t viewId{};
    std::uint64_t conversationId{};
    std::uint64_t nodeId{};
    std::uint64_t choiceCount{};
    std::uint64_t speakerId{};
    std::uint64_t flags{};
};
class DialogueViewStateCollection {
public:
 bool store(DialogueViewState value); bool erase(std::uint64_t id); [[nodiscard]] const DialogueViewState* find(std::uint64_t id) const; [[nodiscard]] const std::vector<DialogueViewState>& all() const noexcept { return rows_; }
private:
 static std::uint64_t idOf(const DialogueViewState& v) noexcept; std::vector<DialogueViewState> rows_;
};
}
