#include "gear/PersonalAssistance.hpp"
#include <algorithm>
#include <cmath>
namespace elysium::gear {
bool PersonalExoskeleton::restore(ExoskeletonState s) {
    const auto& d=s.spec;
    if (!s.owner || !s.item || s.owner==s.item ||
        !std::isfinite(d.energyPerSecond) || d.energyPerSecond<=0 ||
        !std::isfinite(d.carryBonus) || d.carryBonus<0 ||
        !std::isfinite(d.throughputBonus) || d.throughputBonus<0 ||
        !std::isfinite(d.fallReduction) || d.fallReduction<0 || d.fallReduction>1) return false;
    state_=s; return true;
}
AssistanceQuery PersonalExoskeleton::advance(float seconds, SurvivalState& body) const {
    AssistanceQuery q;
    if (!state_.equipped) return q;
    if (!std::isfinite(seconds) || seconds<=0 || !std::isfinite(body.energy) || body.energy<0) {
        q.reason=AssistanceReason::InvalidRequest; return q;
    }
    const double required=double(seconds)*state_.spec.energyPerSecond;
    if (body.energy<=0) { q.reason=AssistanceReason::NoEnergy; return q; }
    q.energySpent=float(std::min(required,double(body.energy)));
    q.poweredFraction=float(q.energySpent/required);
    body.energy=std::max(0.0f,body.energy-q.energySpent);
    q.carryBonus=state_.spec.carryBonus*q.poweredFraction;
    q.throughputMultiplier=1+state_.spec.throughputBonus*q.poweredFraction;
    q.fallReduction=state_.spec.fallReduction*q.poweredFraction;
    q.reason=AssistanceReason::Ready;
    // Queries only: the mining authority still owns extraction and provenance.
    return q;
}
bool PersonalHarness::registerBuffer(HarnessAccount a) {
    if (!a.owner || !a.item || a.owner==a.item || a.stored>a.capacity ||
        static_cast<unsigned>(a.kind)>2) return false;
    for (const auto& [key,existing]:buffers_)
        if (existing.item==a.item && existing.owner!=a.owner) return false;
    return buffers_.emplace(Key{a.item,a.kind},a).second;
}
BufferTransfer PersonalHarness::transfer(std::uint64_t actor,std::uint64_t from,std::uint64_t to,
                                        PersonalBuffer kind,std::uint64_t amount) {
    auto a=buffers_.find({from,kind}), b=buffers_.find({to,kind});
    if (!amount || from==to) return {};
    if (a==buffers_.end() || b==buffers_.end()) return {AssistanceReason::UnknownBuffer,0};
    if (a->second.owner!=actor || b->second.owner!=actor) return {AssistanceReason::ForeignOwner,0};
    if (a->second.stored<amount || b->second.capacity-b->second.stored<amount)
        return {AssistanceReason::Capacity,0};
    a->second.stored-=amount; b->second.stored+=amount;
    return {AssistanceReason::Ready,amount};
}
BufferTransfer PersonalHarness::consume(std::uint64_t actor,std::uint64_t item,PersonalBuffer kind,std::uint64_t amount) {
    auto it=buffers_.find({item,kind});
    if (it==buffers_.end()) return {AssistanceReason::UnknownBuffer,0};
    if (it->second.owner!=actor) return {AssistanceReason::ForeignOwner,0};
    if (!amount) return {};
    if (it->second.stored<amount) return {AssistanceReason::Capacity,0};
    it->second.stored-=amount; return {AssistanceReason::Ready,amount};
}
bool PersonalHarness::resize(std::uint64_t actor,std::uint64_t item,PersonalBuffer kind,std::uint64_t capacity) {
    auto it=buffers_.find({item,kind});
    if (it==buffers_.end() || it->second.owner!=actor || capacity<it->second.stored) return false;
    it->second.capacity=capacity; return true;
}
std::vector<HarnessAccount> PersonalHarness::snapshot() const {
    std::vector<HarnessAccount> out; for (const auto& [key,value]:buffers_) out.push_back(value); return out;
}
bool PersonalHarness::restore(const std::vector<HarnessAccount>& records) {
    PersonalHarness staged; for (auto a:records) if (!staged.registerBuffer(a)) return false;
    buffers_.swap(staged.buffers_); return true;
}
}
