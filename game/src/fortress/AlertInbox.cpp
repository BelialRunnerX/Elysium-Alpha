#include "fortress/AlertInbox.hpp"

#include <algorithm>

namespace elysium::fortress {

void AlertInbox::publish(AlertRecord alert) {
    const auto key = std::make_pair(alert.category, alert.subject.value);
    const auto it = alerts_.find(key);
    if (it == alerts_.end() || alert.urgency >= it->second.urgency) {
        alerts_[key] = std::move(alert);
    }
}

void AlertInbox::clearSubject(StableId subject, std::string_view category) {
    auto it = alerts_.begin();
    while (it != alerts_.end()) {
        if (it->first.second == subject.value && (category.empty() || it->first.first == category)) {
            it = alerts_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<AlertRecord> AlertInbox::ordered() const {
    std::vector<AlertRecord> result;
    result.reserve(alerts_.size());
    for (const auto& [key, alert] : alerts_) {
        (void)key;
        result.push_back(alert);
    }
    std::sort(result.begin(), result.end(), [](const AlertRecord& a, const AlertRecord& b) {
        if (a.level != b.level) return a.level > b.level;
        if (a.urgency != b.urgency) return a.urgency > b.urgency;
        if (a.category != b.category) return a.category < b.category;
        return a.subject.value < b.subject.value;
    });
    return result;
}

} // namespace elysium::fortress
