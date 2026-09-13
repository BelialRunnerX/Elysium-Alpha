// Intended function: Publish deterministic contract offers, availability windows, faction requirements, reward previews, and acceptance claims.
#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace elysium::strategy {

struct ContractOffer {
    std::uint64_t offerId{};
    std::uint64_t contractType{};
    std::uint64_t issuerId{};
    std::uint64_t rewardValue{};
    std::uint64_t deadlineTick{};
    std::uint64_t flags{};
};

class ContractOfferStore {
public:
    bool upsert(ContractOffer value);
    bool erase(std::uint64_t stableId);
    [[nodiscard]] const ContractOffer* find(std::uint64_t stableId) const;
    [[nodiscard]] std::vector<ContractOffer> ordered() const;
    [[nodiscard]] std::size_t size() const noexcept { return records_.size(); }
private:
    static std::uint64_t keyOf(const ContractOffer& value) noexcept;
    std::vector<ContractOffer> records_;
};

} // namespace elysium::strategy
