#pragma once

#include "fortress/Systems.hpp"

#include <map>
#include <vector>

namespace elysium::fortress {

class AlertInbox {
public:
    void publish(AlertRecord alert);
    void clearSubject(StableId subject, std::string_view category = {});
    [[nodiscard]] std::vector<AlertRecord> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return alerts_.size(); }

private:
    std::map<std::pair<std::string, std::uint64_t>, AlertRecord> alerts_;
};

} // namespace elysium::fortress
