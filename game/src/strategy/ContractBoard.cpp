// Intended function: Publish deterministic contract offers, availability windows, faction requirements, reward previews, and acceptance claims.
#include "ContractBoard.hpp"

namespace elysium::strategy {

std::uint64_t ContractOfferStore::keyOf(const ContractOffer& value) noexcept { return static_cast<std::uint64_t>(value.offerId); }

bool ContractOfferStore::upsert(ContractOffer value) {
    const auto key = keyOf(value);
    if (key == 0) return false;
    auto it = std::lower_bound(records_.begin(), records_.end(), key, [](const ContractOffer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it != records_.end() && keyOf(*it) == key) { *it = value; return true; }
    records_.insert(it, value);
    return true;
}

bool ContractOfferStore::erase(std::uint64_t stableId) {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ContractOffer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    if (it == records_.end() || keyOf(*it) != stableId) return false;
    records_.erase(it);
    return true;
}

const ContractOffer* ContractOfferStore::find(std::uint64_t stableId) const {
    auto it = std::lower_bound(records_.begin(), records_.end(), stableId, [](const ContractOffer& lhs, std::uint64_t rhs) { return keyOf(lhs) < rhs; });
    return it != records_.end() && keyOf(*it) == stableId ? &*it : nullptr;
}

std::vector<ContractOffer> ContractOfferStore::ordered() const { return records_; }

} // namespace elysium::strategy
