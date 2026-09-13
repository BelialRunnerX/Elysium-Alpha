// Intended function: imported render implementation for PresentationRuntime; preserves the agent-authored subsystem contract for later integration/debugging.
#include "render/PresentationRuntime.hpp"

#include <algorithm>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace elysium {
namespace {

std::uint64_t mixFingerprint(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    h *= 1099511628211ULL;
    return h;
}

PresentationCost sanitize(PresentationCost c) {
    c.triangles=std::max(0,c.triangles);
    c.instances=std::max(0,c.instances);
    c.particles=std::max(0,c.particles);
    c.lights=std::max(0,c.lights);
    c.shadowCasters=std::max(0,c.shadowCasters);
    c.drawCalls=std::max(0,c.drawCalls);
    c.effectWorkUnits=std::max(0,c.effectWorkUnits);
    return c;
}

PresentationCost scaledCost(const PresentationCost& c, int numerator, int denominator) {
    auto scale=[&](int v) { return v<=0?0:std::max(1,(v*numerator+denominator-1)/denominator); };
    return {scale(c.triangles),scale(c.instances),scale(c.particles),scale(c.lights),
            scale(c.shadowCasters),scale(c.drawCalls),scale(c.effectWorkUnits)};
}

void addCost(PresentationCost& a, const PresentationCost& b) {
    a.triangles+=b.triangles;
    a.instances+=b.instances;
    a.particles+=b.particles;
    a.lights+=b.lights;
    a.shadowCasters+=b.shadowCasters;
    a.drawCalls+=b.drawCalls;
    a.effectWorkUnits+=b.effectWorkUnits;
}

bool candidateLess(const PresentationCandidate& a, const PresentationCandidate& b) {
    const auto ai=static_cast<int>(a.importance);
    const auto bi=static_cast<int>(b.importance);
    if(ai!=bi) return ai<bi;
    if(a.preserveMeaning!=b.preserveMeaning) return a.preserveMeaning>b.preserveMeaning;
    const float ad=std::isfinite(a.distance)?std::max(0.0f,a.distance):std::numeric_limits<float>::max();
    const float bd=std::isfinite(b.distance)?std::max(0.0f,b.distance):std::numeric_limits<float>::max();
    if(ad!=bd) return ad<bd;
    if(a.stableSourceId!=b.stableSourceId) return a.stableSourceId<b.stableSourceId;
    return a.contentKey<b.contentKey;
}

bool voiceLess(const AudioVoiceCandidate& a, const AudioVoiceCandidate& b) {
    const auto ai=static_cast<int>(a.importance);
    const auto bi=static_cast<int>(b.importance);
    if(ai!=bi) return ai<bi;
    if(a.loop!=b.loop) return a.loop<b.loop; // one-shots win ties over ambient loops
    const float ad=std::isfinite(a.distance)?std::max(0.0f,a.distance):std::numeric_limits<float>::max();
    const float bd=std::isfinite(b.distance)?std::max(0.0f,b.distance):std::numeric_limits<float>::max();
    if(ad!=bd) return ad<bd;
    if(a.stableSourceId!=b.stableSourceId) return a.stableSourceId<b.stableSourceId;
    return a.cueFingerprint<b.cueFingerprint;
}

} // namespace

bool presentationCostFits(const PresentationCost& used,
                          const PresentationCost& add,
                          const PresentationFrameBudget& budget) {
    return used.triangles+add.triangles<=std::max(0,budget.triangles) &&
           used.instances+add.instances<=std::max(0,budget.instances) &&
           used.particles+add.particles<=std::max(0,budget.particles) &&
           used.lights+add.lights<=std::max(0,budget.lights) &&
           used.shadowCasters+add.shadowCasters<=std::max(0,budget.shadowCasters) &&
           used.drawCalls+add.drawCalls<=std::max(0,budget.drawCalls) &&
           used.effectWorkUnits+add.effectWorkUnits<=std::max(0,budget.effectWorkUnits);
}

PresentationFrameSelection selectPresentationFrame(std::vector<PresentationCandidate> candidates,
                                                    const PresentationFrameBudget& budget) {
    if(budget.triangles<0 || budget.instances<0 || budget.particles<0 || budget.lights<0 ||
       budget.shadowCasters<0 || budget.drawCalls<0 || budget.effectWorkUnits<0)
        throw std::invalid_argument("presentation frame budget cannot be negative");

    for(auto& c:candidates) {
        if(!std::isfinite(c.distance) || c.distance<0.0f) c.distance=std::numeric_limits<float>::max();
        c.full=sanitize(c.full);
        c.reduced=sanitize(c.reduced);
        c.proxy=sanitize(c.proxy);
    }
    std::sort(candidates.begin(),candidates.end(),candidateLess);

    PresentationFrameSelection out{};
    out.admissions.reserve(candidates.size());
    std::uint64_t fp=1469598103934665603ULL;

    for(const auto& c:candidates) {
        PresentationAdmission admission{};
        admission.stableSourceId=c.stableSourceId;
        admission.contentKey=c.contentKey;
        admission.quality=PresentationQuality::Culled;

        const auto tryQuality=[&](PresentationQuality q,const PresentationCost& cost)->bool {
            if(!presentationCostFits(out.used,cost,budget)) return false;
            admission.quality=q;
            admission.cost=cost;
            addCost(out.used,cost);
            return true;
        };

        if(tryQuality(PresentationQuality::Full,c.full)) ++out.fullCount;
        else if(tryQuality(PresentationQuality::Reduced,c.reduced)) ++out.reducedCount;
        else if(tryQuality(PresentationQuality::Proxy,c.proxy)) ++out.proxyCount;
        else {
            ++out.culledCount;
            if(c.preserveMeaning) ++out.mandatoryProxyMisses;
        }

        fp=mixFingerprint(fp,c.stableSourceId);
        fp=mixFingerprint(fp,c.contentKey);
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(admission.quality));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(admission.cost.triangles));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(admission.cost.instances));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(admission.cost.particles));
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(admission.cost.drawCalls));
        out.admissions.push_back(admission);
    }
    out.fingerprint=fp;
    return out;
}

PresentationCandidate detailPresentationCandidate(std::uint64_t stableSourceId,
                                                  std::uint64_t contentKey,
                                                  const DetailClusterPacket& packet,
                                                  float distance,
                                                  PresentationImportance importance) {
    PresentationCost full{packet.estimatedTriangles,static_cast<int>(packet.instances.size()),0,0,
                          packet.shadowCasters,packet.instances.empty()?0:1,0};
    PresentationCandidate c{};
    c.stableSourceId=stableSourceId;
    c.contentKey=contentKey;
    c.importance=importance;
    c.distance=distance;
    c.full=full;
    c.reduced=scaledCost(full,1,2);
    c.proxy=scaledCost(full,1,8);
    c.preserveMeaning=false; // detail clusters are explicitly regenerable dressing
    return c;
}

PresentationCandidate effectPresentationCandidate(std::uint64_t stableSourceId,
                                                  std::uint64_t contentKey,
                                                  const EffectPacket& packet,
                                                  float distance,
                                                  PresentationImportance importance) {
    PresentationCost full{packet.instances*12,packet.instances,static_cast<int>(packet.particles.size()),packet.dynamicLights,
                          0,(packet.instances>0 || !packet.particles.empty())?1:0,packet.estimatedWorkUnits};
    PresentationCandidate c{};
    c.stableSourceId=stableSourceId;
    c.contentKey=contentKey;
    c.importance=importance;
    c.distance=distance;
    c.full=full;
    c.reduced=scaledCost(full,1,2);
    c.proxy=scaledCost(full,1,5);
    c.preserveMeaning=importance==PresentationImportance::Critical || importance==PresentationImportance::Strategic;
    return c;
}

PresentationCandidate structurePresentationCandidate(std::uint64_t stableSourceId,
                                                     std::uint64_t contentKey,
                                                     int fullTriangles,
                                                     int fullDrawCalls,
                                                     int fullShadowCasters,
                                                     float distance,
                                                     PresentationImportance importance,
                                                     bool preserveMeaning) {
    PresentationCandidate c{};
    c.stableSourceId=stableSourceId;
    c.contentKey=contentKey;
    c.importance=importance;
    c.distance=distance;
    c.full={std::max(0,fullTriangles),0,0,0,std::max(0,fullShadowCasters),std::max(0,fullDrawCalls),0};
    c.reduced=scaledCost(c.full,1,2);
    c.proxy=scaledCost(c.full,1,12);
    c.preserveMeaning=preserveMeaning;
    return c;
}

AudioVoiceSelection selectAudioVoices(std::vector<AudioVoiceCandidate> candidates, int maxVoices) {
    if(maxVoices<0) throw std::invalid_argument("audio voice budget cannot be negative");
    for(auto& c:candidates) {
        if(!std::isfinite(c.distance) || c.distance<0.0f) c.distance=std::numeric_limits<float>::max();
        if(!std::isfinite(c.gain)) c.gain=0.0f;
        c.gain=std::clamp(c.gain,0.0f,1.0f);
    }
    std::sort(candidates.begin(),candidates.end(),voiceLess);
    AudioVoiceSelection out{};
    const std::size_t keep=std::min<std::size_t>(candidates.size(),static_cast<std::size_t>(maxVoices));
    out.voices.assign(candidates.begin(),candidates.begin()+static_cast<std::ptrdiff_t>(keep));
    out.dropped=static_cast<int>(candidates.size()-keep);
    std::uint64_t fp=1469598103934665603ULL;
    for(const auto& c:out.voices) {
        fp=mixFingerprint(fp,c.stableSourceId);
        fp=mixFingerprint(fp,c.cueFingerprint);
        fp=mixFingerprint(fp,static_cast<std::uint64_t>(c.importance));
        fp=mixFingerprint(fp,std::bit_cast<std::uint32_t>(c.distance));
        fp=mixFingerprint(fp,std::bit_cast<std::uint32_t>(c.gain));
        fp=mixFingerprint(fp,c.loop?1ULL:0ULL);
    }
    out.fingerprint=fp;
    return out;
}

} // namespace elysium
