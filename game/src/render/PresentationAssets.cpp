// Intended function: imported render implementation for PresentationAssets; preserves the agent-authored subsystem contract for later integration/debugging.
#include "render/PresentationAssets.hpp"

#include "core/Determinism.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace elysium {
namespace {

constexpr std::uint64_t kMaterialLabel = 0x4D4154455249414CULL;
constexpr std::uint64_t kDetailLabel = 0x44455441494C3031ULL;
constexpr std::uint64_t kRigLabel = 0x524947504C414E31ULL;
constexpr std::uint64_t kEffectLabel = 0x4546464543543031ULL;
constexpr std::uint64_t kAudioLabel = 0x415544494F303031ULL;

std::uint8_t clampByte(int value) {
    return static_cast<std::uint8_t>(std::clamp(value,0,255));
}

int luminance(Color4u c) {
    return (54*static_cast<int>(c.r) + 183*static_cast<int>(c.g) + 19*static_cast<int>(c.b)) / 256;
}

std::uint64_t fnvMix(std::uint64_t h, std::uint64_t value) {
    h ^= value;
    h *= 1099511628211ULL;
    return h;
}

std::uint64_t colorBits(Color4u c) {
    return static_cast<std::uint64_t>(c.r) |
           (static_cast<std::uint64_t>(c.g)<<8U) |
           (static_cast<std::uint64_t>(c.b)<<16U) |
           (static_cast<std::uint64_t>(c.a)<<24U);
}

int signedNoise(std::uint64_t seed, int x, int y, std::uint64_t label) {
    const std::uint64_t h=hashCoords(seed,x,y,0,label);
    return static_cast<int>((h>>56U)&0xFFU)-128;
}

int floorDiv(int value, int divisor) {
    if (value>=0) return value/divisor;
    return -(((-value)+divisor-1)/divisor);
}

int smoothFixedNoise(std::uint64_t seed, int x, int y, int periodX, int periodY, std::uint64_t label) {
    periodX=std::max(1,periodX);
    periodY=std::max(1,periodY);
    const int gx=floorDiv(x,periodX);
    const int gy=floorDiv(y,periodY);
    const int lx=x-gx*periodX;
    const int ly=y-gy*periodY;
    const int wx=(lx*256)/periodX;
    const int wy=(ly*256)/periodY;
    const int n00=signedNoise(seed,gx,gy,label);
    const int n10=signedNoise(seed,gx+1,gy,label);
    const int n01=signedNoise(seed,gx,gy+1,label);
    const int n11=signedNoise(seed,gx+1,gy+1,label);
    const int top=(n00*(256-wx)+n10*wx)/256;
    const int bottom=(n01*(256-wx)+n11*wx)/256;
    return (top*(256-wy)+bottom*wy)/256;
}

Color4u applyValueDelta(Color4u base, int delta) {
    return {clampByte(static_cast<int>(base.r)+delta),
            clampByte(static_cast<int>(base.g)+delta),
            clampByte(static_cast<int>(base.b)+delta),
            base.a};
}

Color4u blend(Color4u a, Color4u b, int share256) {
    share256=std::clamp(share256,0,256);
    const int inv=256-share256;
    return {clampByte((static_cast<int>(a.r)*inv+static_cast<int>(b.r)*share256)/256),
            clampByte((static_cast<int>(a.g)*inv+static_cast<int>(b.g)*share256)/256),
            clampByte((static_cast<int>(a.b)*inv+static_cast<int>(b.b)*share256)/256),
            a.a};
}

std::uint64_t fieldFingerprint(const GeneratedMaterialField& field) {
    std::uint64_t h=1469598103934665603ULL;
    h=fnvMix(h,static_cast<std::uint64_t>(field.width));
    h=fnvMix(h,static_cast<std::uint64_t>(field.height));
    for(std::size_t i=0;i<field.albedo.size();++i) {
        h=fnvMix(h,colorBits(field.albedo[i]));
        h=fnvMix(h,field.roughness[i]);
        h=fnvMix(h,field.emissive[i]);
    }
    return h;
}

MaterialFieldMetrics analyzeField(const GeneratedMaterialField& field) {
    MaterialFieldMetrics out{};
    if(field.albedo.empty()) return out;
    long long lumaSum=0;
    long long roughnessSum=0;
    long long horizontal=0;
    long long vertical=0;
    int hPairs=0;
    int vPairs=0;
    int minL=255;
    int maxL=0;
    for(int y=0;y<field.height;++y) {
        for(int x=0;x<field.width;++x) {
            const std::size_t i=static_cast<std::size_t>(x+y*field.width);
            const int l=luminance(field.albedo[i]);
            lumaSum+=l;
            roughnessSum+=field.roughness[i];
            minL=std::min(minL,l);
            maxL=std::max(maxL,l);
            if(field.emissive[i]>0) ++out.emissivePixels;
            if(x+1<field.width) {
                horizontal+=std::abs(l-luminance(field.albedo[i+1]));
                ++hPairs;
            }
            if(y+1<field.height) {
                vertical+=std::abs(l-luminance(field.albedo[i+static_cast<std::size_t>(field.width)]));
                ++vPairs;
            }
        }
    }
    const float count=static_cast<float>(field.albedo.size());
    out.meanLuminance=static_cast<float>(lumaSum)/count;
    out.luminanceRange=maxL-minL;
    out.horizontalNeighborDelta=hPairs>0?static_cast<float>(horizontal)/static_cast<float>(hPairs):0.0f;
    out.verticalNeighborDelta=vPairs>0?static_cast<float>(vertical)/static_cast<float>(vPairs):0.0f;
    out.meanRoughness=static_cast<float>(roughnessSum)/count;
    return out;
}

struct DetailDefinition {
    DetailPrimitive primitive;
    int fullCount;
    int reducedDivisor;
    bool survivesSilhouette;
    int triangleCost;
    bool castsShadow;
    Color4u base;
};

constexpr std::array<DetailDefinition,static_cast<std::size_t>(DetailClusterKind::Count)> kDetails{{
    {DetailPrimitive::Blade, 28,4,true, 2,true, {74,123,70,255}},
    {DetailPrimitive::Blade, 12,3,true, 4,true, {170,94,139,255}},
    {DetailPrimitive::Blade, 18,3,true, 2,true, {102,132,83,255}},
    {DetailPrimitive::Segment,10,2,false,8,true, {91,68,47,255}},
    {DetailPrimitive::Box, 12,3,false,12,true, {105,106,110,255}},
    {DetailPrimitive::Box, 22,5,false,12,false,{116,116,119,255}},
    {DetailPrimitive::Prism,9,2,true,12,true,  {97,219,183,255}},
    {DetailPrimitive::Prism,8,2,true,12,true,  {178,218,232,210}},
    {DetailPrimitive::Plate,10,2,true,8,true,  {117,163,91,255}},
    {DetailPrimitive::Segment,13,3,true,10,true,{109,166,158,255}},
    {DetailPrimitive::Segment,8,2,false,8,true, {63,72,76,255}},
    {DetailPrimitive::Segment,7,2,false,12,true,{80,92,97,255}},
    {DetailPrimitive::Plate,12,3,false,8,true,  {25,31,33,255}},
    {DetailPrimitive::Plate,11,2,false,8,true,  {105,91,78,255}},
    {DetailPrimitive::Plate,8,3,false,4,false,  {146,132,116,190}},
    {DetailPrimitive::Plate,9,3,false,4,false,  {218,230,235,210}},
    {DetailPrimitive::Decal,7,2,false,2,false,  {61,52,49,180}},
    {DetailPrimitive::Link,14,2,false,10,true,  {74,80,84,255}},
}};

int detailCountForLod(const DetailDefinition& def, DetailLod lod, float density) {
    if(density<=0.0f) return 0;
    const int full=std::max(1,static_cast<int>(std::lround(static_cast<float>(def.fullCount)*std::clamp(density,0.05f,4.0f))));
    switch(lod) {
        case DetailLod::Full: return full;
        case DetailLod::Reduced: return std::max(1,full/std::max(2,def.reducedDivisor));
        case DetailLod::Silhouette: return def.survivesSilhouette?1:0;
        case DetailLod::Culled: return 0;
    }
    return 0;
}

float unitHash(std::uint64_t seed, int index, int lane, std::uint64_t label) {
    return hash01(seed,index,lane,0,label);
}

std::uint64_t detailFingerprint(const DetailClusterPacket& packet) {
    std::uint64_t h=1469598103934665603ULL;
    h=fnvMix(h,static_cast<std::uint64_t>(packet.kind));
    h=fnvMix(h,static_cast<std::uint64_t>(packet.lod));
    for(const auto& instance:packet.instances) {
        h=fnvMix(h,static_cast<std::uint64_t>(instance.primitive));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.position.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.position.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.position.z));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.scale.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.scale.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(instance.scale.z));
        h=fnvMix(h,colorBits(instance.color));
    }
    return h;
}

std::uint64_t archetypeHash(std::uint64_t domain, std::uint64_t value) {
    return mix64(domain ^ mix64(value));
}

constexpr std::array<VisualLanguageProfile,3> kVisualLanguages{{
    {FactionVisualStyle::Neutral,{88,94,98,255},{130,134,136,255},{122,172,174,255},true,0.10f,0.25f,0.10f},
    {FactionVisualStyle::Imperial,{18,23,24,255},{33,52,45,255},{45,207,139,255},true,0.04f,0.08f,0.35f},
    {FactionVisualStyle::UnswornFrontier,{94,82,69,255},{128,114,96,255},{208,136,73,255},false,0.72f,0.85f,0.12f},
}};

constexpr std::array<MachineVisualDefinition,20> kMachineVisuals{{
    {"machine/burner_generator",FactionVisualStyle::UnswornFrontier,420,true,true},
    {"machine/battery_bank",FactionVisualStyle::Neutral,360,false,true},
    {"machine/atmosphere_unit",FactionVisualStyle::Neutral,480,true,true},
    {"machine/storage_crate",FactionVisualStyle::UnswornFrontier,180,false,false},
    {"machine/airlock_controller",FactionVisualStyle::Neutral,220,false,true},
    {"machine/sensor_mast",FactionVisualStyle::Imperial,300,true,true},
    {"machine/turret",FactionVisualStyle::Imperial,620,true,true},
    {"machine/shield_pylon",FactionVisualStyle::Imperial,520,true,true},
    {"machine/logic_controller",FactionVisualStyle::Imperial,280,false,true},
    {"machine/furnace",FactionVisualStyle::UnswornFrontier,420,true,true},
    {"machine/alloy_crucible",FactionVisualStyle::UnswornFrontier,460,true,true},
    {"machine/refinery",FactionVisualStyle::Neutral,680,true,true},
    {"machine/network_storage",FactionVisualStyle::Imperial,400,false,true},
    {"machine/conveyor",FactionVisualStyle::Neutral,120,true,false},
    {"machine/sorter",FactionVisualStyle::Neutral,180,true,true},
    {"machine/cargo_loader",FactionVisualStyle::Neutral,520,true,true},
    {"machine/crusher",FactionVisualStyle::UnswornFrontier,560,true,true},
    {"machine/chemical_vat",FactionVisualStyle::Neutral,500,true,true},
    {"machine/fabricator",FactionVisualStyle::Imperial,720,true,true},
    {"machine/extractor",FactionVisualStyle::UnswornFrontier,760,true,true},
}};

float phase01(std::uint64_t stableId, float seconds, float rate) {
    const float offset=static_cast<float>((mix64(stableId)>>40U)&0xFFFFU)/65535.0f;
    float phase=std::fmod(offset+seconds*rate,1.0f);
    if(phase<0.0f) phase+=1.0f;
    return phase;
}

VisualStatePacket makeGenericVisualState(std::uint64_t stableId,
                                         std::uint64_t archetype,
                                         FactionVisualStyle style,
                                         bool active,
                                         bool powered,
                                         bool faulted,
                                         float damage01,
                                         float simulationSeconds,
                                         int triangles,
                                         bool emissiveWhenPowered,
                                         bool articulated) {
    damage01=std::clamp(damage01,0.0f,1.0f);
    VisualStatePacket out{};
    out.stableSourceId=stableId;
    out.archetypeKey=archetype;
    out.style=style;
    out.wear=damage01;
    out.estimatedTriangles=triangles;
    out.accent=visualLanguage(style).accent;
    if(faulted) out.state=VisualActivityState::Fault;
    else if(damage01>=0.65f) out.state=VisualActivityState::Damaged;
    else if(active) out.state=VisualActivityState::Active;
    else if(powered) out.state=VisualActivityState::Powered;
    else out.state=VisualActivityState::Idle;
    out.animationPhase=articulated?phase01(stableId,simulationSeconds,active?0.55f:0.08f):0.0f;
    out.emissive=(powered && emissiveWhenPowered)?(active?1.0f:0.45f):0.0f;
    if(faulted) out.emissive=0.22f+0.18f*(out.animationPhase>0.5f?1.0f:0.0f);
    return out;
}

struct RigTemplatePart { RigPartRole role; int parent; Vec3 p; Vec3 e; };

void addRigPart(CreatureRigPacket& out, const RigTemplatePart& t, float scale, float widthScale, float lengthScale) {
    RigPart p{};
    p.role=t.role;
    p.parent=t.parent;
    p.localPosition={t.p.x*widthScale*scale,t.p.y*scale,t.p.z*lengthScale*scale};
    p.halfExtents={std::max(0.03f,t.e.x*widthScale*scale),std::max(0.03f,t.e.y*scale),std::max(0.03f,t.e.z*lengthScale*scale)};
    out.parts.push_back(p);
}

std::uint64_t rigFingerprint(const CreatureRigPacket& rig) {
    std::uint64_t h=1469598103934665603ULL;
    h=fnvMix(h,static_cast<std::uint64_t>(rig.bodyPlan));
    for(const auto& p:rig.parts) {
        h=fnvMix(h,static_cast<std::uint64_t>(p.role));
        h=fnvMix(h,static_cast<std::uint64_t>(p.parent+1));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.localPosition.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.localPosition.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.localPosition.z));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.halfExtents.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.halfExtents.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.halfExtents.z));
    }
    return h;
}

std::uint64_t damageFingerprint(const DamageVisualPacket& packet) {
    std::uint64_t h=1469598103934665603ULL;
    h=fnvMix(h,static_cast<std::uint64_t>(packet.exposedLayers));
    for(const auto& m:packet.marks) {
        h=fnvMix(h,static_cast<std::uint64_t>(m.kind));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(m.offset.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(m.offset.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(m.offset.z));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(m.intensity));
    }
    return h;
}

std::uint64_t effectFingerprint(const EffectPacket& packet) {
    std::uint64_t h=1469598103934665603ULL;
    h=fnvMix(h,static_cast<std::uint64_t>(packet.kind));
    h=fnvMix(h,static_cast<std::uint64_t>(packet.instances));
    h=fnvMix(h,static_cast<std::uint64_t>(packet.dynamicLights));
    for(const auto& p:packet.particles) {
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.position.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.position.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.position.z));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.velocity.x));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.velocity.y));
        h=fnvMix(h,std::bit_cast<std::uint32_t>(p.velocity.z));
        h=fnvMix(h,colorBits(p.color));
    }
    return h;
}

AudioCuePacket makeAudioCue(AudioTimbre timbre,
                            float basePitch,
                            float gain,
                            float lowShare,
                            float pulse,
                            bool loop,
                            std::uint64_t seed,
                            int variations) {
    AudioCuePacket out{};
    out.timbre=timbre;
    const std::uint64_t h=mix64(seed^kAudioLabel^static_cast<std::uint64_t>(timbre));
    out.variation=variations>0?static_cast<int>(h%static_cast<std::uint64_t>(variations)):0;
    const float pitchJitter=(static_cast<float>((h>>16U)&0xFFFFU)/65535.0f-0.5f)*0.10f;
    out.pitchHz=basePitch*(1.0f+pitchJitter);
    out.gain=std::clamp(gain,0.0f,1.0f);
    out.lowFrequencyShare=std::clamp(lowShare,0.0f,1.0f);
    out.pulseHz=std::max(0.0f,pulse);
    out.loop=loop;
    std::uint64_t fp=1469598103934665603ULL;
    fp=fnvMix(fp,static_cast<std::uint64_t>(out.timbre));
    fp=fnvMix(fp,std::bit_cast<std::uint32_t>(out.pitchHz));
    fp=fnvMix(fp,std::bit_cast<std::uint32_t>(out.gain));
    fp=fnvMix(fp,std::bit_cast<std::uint32_t>(out.lowFrequencyShare));
    fp=fnvMix(fp,std::bit_cast<std::uint32_t>(out.pulseHz));
    fp=fnvMix(fp,static_cast<std::uint64_t>(out.loop));
    fp=fnvMix(fp,static_cast<std::uint64_t>(out.variation));
    out.fingerprint=fp;
    return out;
}

} // namespace

GeneratedMaterialField generateMaterialField(const MaterialFieldSpec& spec) {
    if(spec.width<=0 || spec.height<=0 || spec.width>2048 || spec.height>2048)
        throw std::invalid_argument("material field dimensions must be within 1..2048");
    GeneratedMaterialField out{};
    out.width=spec.width;
    out.height=spec.height;
    const std::size_t count=static_cast<std::size_t>(spec.width)*static_cast<std::size_t>(spec.height);
    out.albedo.resize(count);
    out.roughness.resize(count);
    out.emissive.resize(count);
    const int amplitude=std::max(1,static_cast<int>(spec.variation));
    const std::uint64_t profileLabel=kMaterialLabel^static_cast<std::uint64_t>(spec.profile)*0x9E3779B97F4A7C15ULL;

    for(int y=0;y<spec.height;++y) {
        for(int x=0;x<spec.width;++x) {
            int n=0;
            int rough=145;
            int emissive=0;
            switch(spec.profile) {
                case MaterialFieldProfile::Stone:
                    n=(3*smoothFixedNoise(spec.seed,x,y,9,3,profileLabel)+smoothFixedNoise(spec.seed,x,y,3,2,profileLabel+1))/4;
                    rough=190+smoothFixedNoise(spec.seed,x,y,5,4,profileLabel+2)/8;
                    break;
                case MaterialFieldProfile::Metal:
                    n=(3*smoothFixedNoise(spec.seed,x,y,12,8,profileLabel)+signedNoise(spec.seed,x/2,y,profileLabel+1))/4;
                    rough=92+smoothFixedNoise(spec.seed,x,y,14,5,profileLabel+2)/10;
                    break;
                case MaterialFieldProfile::ClothFiber: {
                    const int warp=smoothFixedNoise(spec.seed,x,y,8,2,profileLabel);
                    const int weft=smoothFixedNoise(spec.seed,x,y,2,8,profileLabel+1);
                    n=(warp-weft)/2;
                    rough=205+(std::abs(warp)+std::abs(weft))/12;
                    break;
                }
                case MaterialFieldProfile::SmoothCeramic:
                    n=smoothFixedNoise(spec.seed,x,y,16,16,profileLabel)/2;
                    rough=54+std::abs(smoothFixedNoise(spec.seed,x,y,20,20,profileLabel+1))/12;
                    break;
                case MaterialFieldProfile::Strand:
                    n=(3*smoothFixedNoise(spec.seed,x,y,2,11,profileLabel)+smoothFixedNoise(spec.seed,x,y,2,4,profileLabel+1))/4;
                    rough=176+smoothFixedNoise(spec.seed,x,y,4,12,profileLabel+2)/10;
                    break;
                case MaterialFieldProfile::Corroded: {
                    const int base=smoothFixedNoise(spec.seed,x,y,10,7,profileLabel);
                    const int patch=smoothFixedNoise(spec.seed,x,y,18,11,profileLabel+1);
                    n=(base+patch)/2;
                    rough=185+std::abs(patch)/3;
                    break;
                }
                case MaterialFieldProfile::IrradiatedExotic: {
                    const int base=smoothFixedNoise(spec.seed,x,y,13,13,profileLabel)/2;
                    const int fracture=std::abs(signedNoise(spec.seed,x/2,y/2,profileLabel+1));
                    const bool glowing=fracture>112 && ((x+y+static_cast<int>(spec.seed&7U))%5<=1);
                    n=base+(glowing?96:0);
                    rough=70+std::abs(base)/4;
                    emissive=glowing?180+fracture/3:0;
                    break;
                }
                case MaterialFieldProfile::Count:
                    throw std::invalid_argument("invalid material field profile");
            }
            const int delta=(n*amplitude)/128;
            Color4u c=applyValueDelta(spec.baseColor,delta);
            if(spec.profile==MaterialFieldProfile::Corroded) {
                const int patch=smoothFixedNoise(spec.seed,x,y,18,11,profileLabel+1);
                if(patch>28) c=blend(c,{142,78,48,spec.baseColor.a},std::min(96,patch));
            }
            if(spec.profile==MaterialFieldProfile::IrradiatedExotic && emissive>0)
                c=blend(c,{84,255,170,spec.baseColor.a},130);
            const std::size_t i=static_cast<std::size_t>(x+y*spec.width);
            out.albedo[i]=c;
            out.roughness[i]=clampByte(rough);
            out.emissive[i]=clampByte(emissive);
        }
    }
    out.metrics=analyzeField(out);
    out.fingerprint=fieldFingerprint(out);
    return out;
}

const char* materialFieldProfileName(MaterialFieldProfile profile) {
    switch(profile) {
        case MaterialFieldProfile::Stone: return "stone";
        case MaterialFieldProfile::Metal: return "metal";
        case MaterialFieldProfile::ClothFiber: return "cloth_fiber";
        case MaterialFieldProfile::SmoothCeramic: return "smooth_ceramic";
        case MaterialFieldProfile::Strand: return "strand";
        case MaterialFieldProfile::Corroded: return "corroded";
        case MaterialFieldProfile::IrradiatedExotic: return "irradiated_exotic";
        case MaterialFieldProfile::Count: break;
    }
    return "invalid";
}

const VisualLanguageProfile& visualLanguage(FactionVisualStyle style) {
    const auto i=static_cast<std::size_t>(style);
    if(i>=kVisualLanguages.size()) throw std::out_of_range("invalid visual language");
    return kVisualLanguages[i];
}

DetailLod detailLodForDistance(DetailClusterKind kind, float distance) {
    if(distance<0.0f) distance=0.0f;
    if(distance<20.0f) return DetailLod::Full;
    if(distance<80.0f) return DetailLod::Reduced;
    if(distance<300.0f) {
        const auto i=static_cast<std::size_t>(kind);
        if(i<kDetails.size() && kDetails[i].survivesSilhouette) return DetailLod::Silhouette;
    }
    return DetailLod::Culled;
}

DetailClusterPacket generateDetailCluster(const DetailClusterSpec& spec) {
    const auto i=static_cast<std::size_t>(spec.kind);
    if(i>=kDetails.size()) throw std::invalid_argument("invalid detail cluster kind");
    const auto& def=kDetails[i];
    DetailClusterPacket out{};
    out.kind=spec.kind;
    out.lod=detailLodForDistance(spec.kind,spec.observerDistance);
    const int count=detailCountForLod(def,out.lod,spec.density);
    out.instances.reserve(static_cast<std::size_t>(count));
    for(int index=0;index<count;++index) {
        const float angle=unitHash(spec.seed,index,0,kDetailLabel+static_cast<std::uint64_t>(spec.kind))*6.28318530718f;
        const float radial=std::sqrt(unitHash(spec.seed,index,1,kDetailLabel+3))*std::max(0.05f,spec.radius);
        const float height=(unitHash(spec.seed,index,2,kDetailLabel+5)-0.5f)*0.16f*spec.radius;
        const float size=0.65f+0.75f*unitHash(spec.seed,index,3,kDetailLabel+7);
        DetailInstance instance{};
        instance.primitive=def.primitive;
        instance.position=spec.origin+Vec3{std::cos(angle)*radial,height,std::sin(angle)*radial};
        const float vertical=(def.primitive==DetailPrimitive::Blade || def.primitive==DetailPrimitive::Segment || def.primitive==DetailPrimitive::Link)?1.6f:0.75f;
        instance.scale={0.18f*size*spec.radius,0.18f*size*vertical*spec.radius,0.18f*size*spec.radius};
        if(out.lod==DetailLod::Silhouette) instance.scale*=2.4f;
        instance.rotation={0.0f,angle,0.0f};
        const int valueShift=static_cast<int>(unitHash(spec.seed,index,4,kDetailLabel+11)*30.0f)-15;
        instance.color=applyValueDelta(def.base,valueShift);
        if(spec.kind==DetailClusterKind::PanelGreeble && (index%4)==0)
            instance.color=visualLanguage(FactionVisualStyle::Imperial).accent;
        if(spec.kind==DetailClusterKind::FrontierPatch && (index%5)==0)
            instance.color=visualLanguage(FactionVisualStyle::UnswornFrontier).accent;
        instance.castsShadow=def.castsShadow && out.lod!=DetailLod::Silhouette;
        out.instances.push_back(instance);
        out.estimatedTriangles+=def.triangleCost;
        if(instance.castsShadow) ++out.shadowCasters;
    }
    out.materialVariants=out.instances.empty()?0:std::min(4,1+count/5);
    out.fingerprint=detailFingerprint(out);
    return out;
}

const char* detailClusterKindName(DetailClusterKind kind) {
    static constexpr std::array<const char*,static_cast<std::size_t>(DetailClusterKind::Count)> names{{
        "grass_tuft","flower_cluster","reed","root_bundle","loose_rubble","pebble_scatter",
        "crystal_spray","ice_shard","fungal_fan","coral_cluster","cable_loom","pipe_valve_cluster",
        "panel_greeble","frontier_patch","dust_drift","snow_lip","scorch_detail","hanging_chain"
    }};
    const auto i=static_cast<std::size_t>(kind);
    return i<names.size()?names[i]:"invalid";
}

const MachineVisualDefinition& machineVisualDefinition(MachineType type) {
    const auto i=static_cast<std::size_t>(type);
    if(i>=kMachineVisuals.size()) throw std::out_of_range("invalid machine visual type");
    return kMachineVisuals[i];
}

VisualStatePacket buildMachineVisualState(std::uint64_t stableId,
                                          MachineType type,
                                          bool enabled,
                                          bool powered,
                                          bool active,
                                          bool faulted,
                                          float damage01,
                                          float simulationSeconds) {
    const auto& def=machineVisualDefinition(type);
    const bool effectiveActive=enabled && powered && active && !faulted;
    return makeGenericVisualState(stableId,
                                  archetypeHash(0x4D414348494E4500ULL,static_cast<std::uint64_t>(type)),
                                  def.style,effectiveActive,powered,faulted,damage01,simulationSeconds,
                                  def.baseTriangles,def.emissiveWhenPowered,def.articulated);
}

VisualStatePacket buildPropVisualState(std::uint64_t stableId,
                                       std::uint64_t visualArchetype,
                                       FactionVisualStyle style,
                                       bool active,
                                       float condition01,
                                       float simulationSeconds) {
    const float damage=1.0f-std::clamp(condition01,0.0f,1.0f);
    return makeGenericVisualState(stableId,archetypeHash(0x50524F5000000000ULL,visualArchetype),style,
                                  active,active,false,damage,simulationSeconds,180,true,active);
}

VisualStatePacket buildStructureVisualState(std::uint64_t stableId,
                                            std::uint64_t visualArchetype,
                                            FactionVisualStyle style,
                                            float damage01,
                                            bool powered,
                                            float simulationSeconds) {
    return makeGenericVisualState(stableId,archetypeHash(0x5354525543540000ULL,visualArchetype),style,
                                  false,powered,false,damage01,simulationSeconds,480,powered,false);
}

CreatureRigPacket generateCreatureRig(const CreatureRigSpec& spec) {
    if(spec.scale<=0.0f || !std::isfinite(spec.scale)) throw std::invalid_argument("creature rig scale must be positive and finite");
    CreatureRigPacket out{};
    out.bodyPlan=spec.bodyPlan;
    const float variation=std::clamp(spec.silhouetteVariation,0.0f,0.5f);
    const float widthScale=1.0f+(hash01(spec.seed,1,0,0,kRigLabel)-0.5f)*2.0f*variation;
    const float lengthScale=1.0f+(hash01(spec.seed,2,0,0,kRigLabel)-0.5f)*2.0f*variation;

    auto add=[&](RigPartRole role,int parent,Vec3 p,Vec3 e){addRigPart(out,{role,parent,p,e},spec.scale,widthScale,lengthScale);};
    switch(spec.bodyPlan) {
        case CreatureBodyPlan::Quadruped:
            add(RigPartRole::Root,-1,{0,0,0},{0.62f,0.46f,0.92f});
            add(RigPartRole::Head,0,{0,0.18f,1.08f},{0.38f,0.34f,0.40f});
            add(RigPartRole::Limb,0,{-0.45f,-0.65f,0.55f},{0.16f,0.58f,0.16f});
            add(RigPartRole::Limb,0,{0.45f,-0.65f,0.55f},{0.16f,0.58f,0.16f});
            add(RigPartRole::Limb,0,{-0.45f,-0.65f,-0.55f},{0.17f,0.62f,0.17f});
            add(RigPartRole::Limb,0,{0.45f,-0.65f,-0.55f},{0.17f,0.62f,0.17f});
            add(RigPartRole::Tail,0,{0,0.05f,-1.20f},{0.12f,0.12f,0.68f});
            break;
        case CreatureBodyPlan::Biped:
            add(RigPartRole::Root,-1,{0,0,0},{0.52f,0.78f,0.34f});
            add(RigPartRole::Head,0,{0,1.02f,0.02f},{0.34f,0.36f,0.32f});
            add(RigPartRole::Limb,0,{-0.72f,0.20f,0},{0.18f,0.64f,0.18f});
            add(RigPartRole::Limb,0,{0.72f,0.20f,0},{0.18f,0.64f,0.18f});
            add(RigPartRole::Limb,0,{-0.30f,-1.02f,0},{0.22f,0.82f,0.24f});
            add(RigPartRole::Limb,0,{0.30f,-1.02f,0},{0.22f,0.82f,0.24f});
            add(RigPartRole::Armor,0,{0,0.15f,-0.38f},{0.58f,0.44f,0.12f});
            break;
        case CreatureBodyPlan::Hexapod:
            add(RigPartRole::Root,-1,{0,0,0},{0.68f,0.42f,0.86f});
            add(RigPartRole::Head,0,{0,0.10f,1.06f},{0.34f,0.28f,0.36f});
            for(int pair=0;pair<3;++pair) {
                const float z=0.62f-static_cast<float>(pair)*0.62f;
                add(RigPartRole::Limb,0,{-0.66f,-0.52f,z},{0.15f,0.48f,0.17f});
                add(RigPartRole::Limb,0,{0.66f,-0.52f,z},{0.15f,0.48f,0.17f});
            }
            add(RigPartRole::Armor,0,{0,0.42f,-0.05f},{0.62f,0.16f,0.72f});
            break;
        case CreatureBodyPlan::Serpentine:
            add(RigPartRole::Root,-1,{0,0,0},{0.34f,0.34f,0.70f});
            for(int segment=1;segment<=6;++segment)
                add(segment==6?RigPartRole::Tail:RigPartRole::Torso,segment-1,{0,0,-0.72f},{0.31f-0.025f*segment,0.30f-0.02f*segment,0.44f});
            add(RigPartRole::Head,0,{0,0.08f,0.82f},{0.42f,0.30f,0.46f});
            break;
        case CreatureBodyPlan::Flyer:
            add(RigPartRole::Root,-1,{0,0,0},{0.45f,0.36f,0.62f});
            add(RigPartRole::Head,0,{0,0.12f,0.76f},{0.28f,0.25f,0.30f});
            add(RigPartRole::Wing,0,{-0.98f,0.08f,-0.02f},{0.74f,0.08f,0.42f});
            add(RigPartRole::Wing,0,{0.98f,0.08f,-0.02f},{0.74f,0.08f,0.42f});
            add(RigPartRole::Tail,0,{0,0,-0.88f},{0.18f,0.12f,0.52f});
            add(RigPartRole::Limb,0,{-0.24f,-0.48f,0.05f},{0.11f,0.32f,0.11f});
            add(RigPartRole::Limb,0,{0.24f,-0.48f,0.05f},{0.11f,0.32f,0.11f});
            break;
        case CreatureBodyPlan::Floater:
            add(RigPartRole::Root,-1,{0,0,0},{0.72f,0.46f,0.72f});
            add(RigPartRole::Sensor,0,{0,0.45f,0.40f},{0.22f,0.18f,0.22f});
            add(RigPartRole::Fin,0,{-0.72f,0,-0.05f},{0.34f,0.08f,0.46f});
            add(RigPartRole::Fin,0,{0.72f,0,-0.05f},{0.34f,0.08f,0.46f});
            add(RigPartRole::Tail,0,{0,-0.15f,-0.86f},{0.18f,0.22f,0.50f});
            add(RigPartRole::Sensor,0,{-0.24f,-0.48f,0.18f},{0.08f,0.36f,0.08f});
            add(RigPartRole::Sensor,0,{0.24f,-0.48f,0.18f},{0.08f,0.36f,0.08f});
            break;
        case CreatureBodyPlan::Count:
            throw std::invalid_argument("invalid creature body plan");
    }
    out.estimatedTriangles=static_cast<int>(out.parts.size())*12;
    out.fingerprint=rigFingerprint(out);
    return out;
}

const char* creatureBodyPlanName(CreatureBodyPlan plan) {
    static constexpr std::array<const char*,static_cast<std::size_t>(CreatureBodyPlan::Count)> names{{
        "quadruped","biped","hexapod","serpentine","flyer","floater"
    }};
    const auto i=static_cast<std::size_t>(plan);
    return i<names.size()?names[i]:"invalid";
}

DamageVisualPacket generateDamageVisuals(std::uint64_t stableSourceId,
                                         float damage01,
                                         bool thermal,
                                         bool corrosive,
                                         int maxMarks) {
    DamageVisualPacket out{};
    damage01=std::clamp(damage01,0.0f,1.0f);
    maxMarks=std::clamp(maxMarks,0,64);
    const int desired=std::min(maxMarks,static_cast<int>(std::ceil(damage01*static_cast<float>(maxMarks))));
    out.marks.reserve(static_cast<std::size_t>(desired));
    for(int i=0;i<desired;++i) {
        DamageVisualMark mark{};
        if(corrosive && i%3==0) mark.kind=DamageVisualKind::Corrosion;
        else if(thermal && i%3==0) mark.kind=DamageVisualKind::Scorch;
        else if(damage01>0.70f && i%4==0) mark.kind=DamageVisualKind::ExposedLayer;
        else if(i%2==0) mark.kind=DamageVisualKind::Crack;
        else mark.kind=DamageVisualKind::Dent;
        mark.offset={hash01(stableSourceId,i,0,0,kEffectLabel)-0.5f,
                     hash01(stableSourceId,i,1,0,kEffectLabel)-0.5f,
                     hash01(stableSourceId,i,2,0,kEffectLabel)-0.5f};
        const float s=0.12f+0.26f*hash01(stableSourceId,i,3,0,kEffectLabel);
        mark.scale={s,s,s};
        mark.intensity=std::clamp(damage01*(0.65f+0.35f*hash01(stableSourceId,i,4,0,kEffectLabel)),0.0f,1.0f);
        if(mark.kind==DamageVisualKind::ExposedLayer) ++out.exposedLayers;
        out.marks.push_back(mark);
    }
    out.fingerprint=damageFingerprint(out);
    return out;
}

EffectPacket generateEffectPacket(PresentationEffectKind kind,
                                  std::uint64_t seed,
                                  Vec3 origin,
                                  float intensity,
                                  float observerDistance,
                                  const EffectBudget& budget) {
    if(static_cast<std::size_t>(kind)>=static_cast<std::size_t>(PresentationEffectKind::Count))
        throw std::invalid_argument("invalid presentation effect kind");
    EffectPacket out{};
    out.kind=kind;
    intensity=std::clamp(intensity,0.0f,2.0f);
    const float distanceScale=observerDistance<20.0f?1.0f:(observerDistance<80.0f?0.45f:(observerDistance<300.0f?0.12f:0.0f));
    int baseParticles=24;
    int baseInstances=3;
    int requestedLights=0;
    Color4u color{180,180,180,220};
    switch(kind) {
        case PresentationEffectKind::DustImpact: baseParticles=18;color={151,135,116,180};break;
        case PresentationEffectKind::StoneDebris: baseParticles=28;baseInstances=8;color={116,118,121,255};break;
        case PresentationEffectKind::MetalSparks: baseParticles=34;requestedLights=1;color={255,188,91,255};break;
        case PresentationEffectKind::Smoke: baseParticles=42;color={78,82,84,150};break;
        case PresentationEffectKind::Fire: baseParticles=48;requestedLights=2;color={255,116,42,230};break;
        case PresentationEffectKind::DecompressionMist: baseParticles=52;color={196,226,235,135};break;
        case PresentationEffectKind::AcidSpray: baseParticles=36;color={143,211,82,210};break;
        case PresentationEffectKind::RadiationPulse: baseParticles=12;requestedLights=1;color={90,222,155,185};break;
        case PresentationEffectKind::PlasmaArc: baseParticles=28;requestedLights=2;color={255,98,58,255};break;
        case PresentationEffectKind::ShieldHit: baseParticles=24;requestedLights=1;color={69,222,170,220};break;
        case PresentationEffectKind::WeatherDust: baseParticles=72;color={158,139,119,120};break;
        case PresentationEffectKind::WeatherSnow: baseParticles=80;color={226,236,242,200};break;
        case PresentationEffectKind::WeatherSpore: baseParticles=58;color={161,111,184,165};break;
        case PresentationEffectKind::RegisterActionSignal: baseParticles=18;requestedLights=2;color=visualLanguage(FactionVisualStyle::Imperial).accent;break;
        case PresentationEffectKind::Count: break;
    }
    const int particleCount=std::clamp(static_cast<int>(std::lround(baseParticles*intensity*distanceScale)),0,std::max(0,budget.maxParticles));
    out.instances=std::clamp(static_cast<int>(std::lround(baseInstances*intensity*distanceScale)),0,std::max(0,budget.maxInstances));
    out.dynamicLights=std::clamp(static_cast<int>(std::ceil(requestedLights*std::min(1.0f,intensity)*distanceScale)),0,std::max(0,budget.maxLights));
    out.particles.reserve(static_cast<std::size_t>(particleCount));
    for(int i=0;i<particleCount;++i) {
        const float a=unitHash(seed,i,0,kEffectLabel+static_cast<std::uint64_t>(kind))*6.28318530718f;
        const float speed=0.25f+2.4f*unitHash(seed,i,1,kEffectLabel+2)*std::max(0.2f,intensity);
        EffectParticle p{};
        p.position=origin+Vec3{(unitHash(seed,i,2,kEffectLabel+4)-0.5f)*0.25f,
                               (unitHash(seed,i,3,kEffectLabel+5)-0.5f)*0.25f,
                               (unitHash(seed,i,4,kEffectLabel+6)-0.5f)*0.25f};
        p.velocity={std::cos(a)*speed,0.3f+speed*unitHash(seed,i,5,kEffectLabel+7),std::sin(a)*speed};
        p.lifetime=0.25f+2.2f*unitHash(seed,i,6,kEffectLabel+8);
        p.size=0.025f+0.14f*unitHash(seed,i,7,kEffectLabel+9);
        p.color=applyValueDelta(color,static_cast<int>(unitHash(seed,i,8,kEffectLabel+10)*22.0f)-11);
        out.particles.push_back(p);
    }
    out.estimatedWorkUnits=particleCount+out.instances*4+out.dynamicLights*16;
    out.fingerprint=effectFingerprint(out);
    return out;
}

const char* presentationEffectName(PresentationEffectKind kind) {
    static constexpr std::array<const char*,static_cast<std::size_t>(PresentationEffectKind::Count)> names{{
        "dust_impact","stone_debris","metal_sparks","smoke","fire","decompression_mist","acid_spray",
        "radiation_pulse","plasma_arc","shield_hit","weather_dust","weather_snow","weather_spore","register_action_signal"
    }};
    const auto i=static_cast<std::size_t>(kind);
    return i<names.size()?names[i]:"invalid";
}

MaterialAudioFamily materialAudioFamily(BlockType material) {
    switch(material) {
        case BlockType::Grass:
        case BlockType::Dirt:
        case BlockType::Regolith: return MaterialAudioFamily::Earth;
        case BlockType::Stone:
        case BlockType::Basalt:
        case BlockType::CoalOre:
        case BlockType::Rubble: return MaterialAudioFamily::Stone;
        case BlockType::Planks: return MaterialAudioFamily::Wood;
        case BlockType::CopperOre:
        case BlockType::TinOre:
        case BlockType::IronOre:
        case BlockType::SteelPlate:
        case BlockType::DoorPanel:
        case BlockType::AirlockPanel: return MaterialAudioFamily::Metal;
        case BlockType::Magma: return MaterialAudioFamily::Fluid;
        case BlockType::RegistryBeacon: return MaterialAudioFamily::Machine;
        case BlockType::Air:
        case BlockType::Count: return MaterialAudioFamily::Organic;
    }
    return MaterialAudioFamily::Organic;
}

AudioCuePacket materialAudioCue(BlockType material, MaterialAudioAction action, std::uint64_t seed) {
    const auto family=materialAudioFamily(material);
    float actionPitch=1.0f;
    float actionGain=0.65f;
    switch(action) {
        case MaterialAudioAction::Mine: actionPitch=0.88f;actionGain=0.78f;break;
        case MaterialAudioAction::Impact: actionPitch=1.0f;actionGain=0.72f;break;
        case MaterialAudioAction::Footstep: actionPitch=1.12f;actionGain=0.48f;break;
        case MaterialAudioAction::Destroy: actionPitch=0.72f;actionGain=0.90f;break;
    }
    switch(family) {
        case MaterialAudioFamily::Earth: return makeAudioCue(AudioTimbre::EarthThud,115.0f*actionPitch,actionGain,0.70f,0,false,seed,5);
        case MaterialAudioFamily::Stone: return makeAudioCue(AudioTimbre::StoneKnock,185.0f*actionPitch,actionGain,0.55f,0,false,seed,6);
        case MaterialAudioFamily::Wood: return makeAudioCue(AudioTimbre::WoodClack,255.0f*actionPitch,actionGain,0.35f,0,false,seed,6);
        case MaterialAudioFamily::Metal: return makeAudioCue(AudioTimbre::MetalRing,410.0f*actionPitch,actionGain,0.42f,0,false,seed,8);
        case MaterialAudioFamily::GlassCeramic: return makeAudioCue(AudioTimbre::GlassChime,690.0f*actionPitch,actionGain,0.18f,0,false,seed,5);
        case MaterialAudioFamily::Fluid: return makeAudioCue(AudioTimbre::FluidSplash,150.0f*actionPitch,actionGain,0.66f,0,false,seed,5);
        case MaterialAudioFamily::Machine: return makeAudioCue(AudioTimbre::MachineHum,92.0f*actionPitch,actionGain,0.82f,1.2f,false,seed,4);
        case MaterialAudioFamily::Organic: return makeAudioCue(AudioTimbre::OrganicRustle,220.0f*actionPitch,actionGain,0.25f,0,false,seed,6);
    }
    return {};
}

AudioCuePacket hazardAudioCue(HazardAudioKind hazard, float intensity, std::uint64_t seed) {
    intensity=std::clamp(intensity,0.0f,1.5f);
    switch(hazard) {
        case HazardAudioKind::Thermal: return makeAudioCue(AudioTimbre::MachineHum,78.0f,0.28f+0.26f*intensity,0.78f,0.6f,true,seed,3);
        case HazardAudioKind::Cryogenic: return makeAudioCue(AudioTimbre::Wind,330.0f,0.25f+0.22f*intensity,0.18f,0.12f,true,seed,4);
        case HazardAudioKind::Corrosive: return makeAudioCue(AudioTimbre::AcidHiss,520.0f,0.22f+0.25f*intensity,0.12f,2.4f,true,seed,5);
        case HazardAudioKind::Radiological: return makeAudioCue(AudioTimbre::RadiationTicks,860.0f,0.18f+0.24f*intensity,0.08f,1.0f+4.0f*intensity,true,seed,7);
        case HazardAudioKind::Pressure: return makeAudioCue(AudioTimbre::PressureCreak,72.0f,0.25f+0.28f*intensity,0.90f,0.18f,true,seed,5);
        case HazardAudioKind::Vacuum: return makeAudioCue(AudioTimbre::VacuumMuffle,52.0f,0.18f+0.20f*intensity,0.94f,0.0f,true,seed,2);
    }
    return {};
}

AudioCuePacket empireAudioCue(EmpireAudioEvent event, float intensity, std::uint64_t seed) {
    intensity=std::clamp(intensity,0.0f,1.5f);
    switch(event) {
        case EmpireAudioEvent::PatrolRelay: return makeAudioCue(AudioTimbre::OrderlyRelay,520.0f,0.40f+0.18f*intensity,0.28f,2.0f,false,seed,4);
        case EmpireAudioEvent::Inspection: return makeAudioCue(AudioTimbre::PrecisionServo,260.0f,0.45f+0.20f*intensity,0.52f,1.0f,false,seed,5);
        case EmpireAudioEvent::RegisterAnnouncement: return makeAudioCue(AudioTimbre::OrderlyRelay,360.0f,0.58f+0.18f*intensity,0.68f,0.75f,false,seed,3);
        case EmpireAudioEvent::WaveStart: return makeAudioCue(AudioTimbre::OrderlyRelay,190.0f,0.66f+0.18f*intensity,0.82f,1.5f,false,seed,4);
        case EmpireAudioEvent::CourtArrival: return makeAudioCue(AudioTimbre::PrecisionServo,130.0f,0.52f+0.18f*intensity,0.76f,0.25f,false,seed,5);
    }
    return {};
}

AudioCuePacket frontierAudioCue(float intensity, std::uint64_t seed) {
    intensity=std::clamp(intensity,0.0f,1.5f);
    return makeAudioCue(AudioTimbre::FrontierRattle,145.0f,0.34f+0.24f*intensity,0.58f,0.42f,true,seed,8);
}

GeneratedAudioClip synthesizeAudioCue(const AudioCuePacket& cue, float seconds, int sampleRate) {
    GeneratedAudioClip out{};
    out.sampleRate=std::clamp(sampleRate,8000,96000);
    out.loop=cue.loop;
    seconds=std::clamp(seconds,0.02f,10.0f);
    const int sampleCount=std::max(1,static_cast<int>(std::lround(seconds*static_cast<float>(out.sampleRate))));
    out.pcm.resize(static_cast<std::size_t>(sampleCount));

    // Quantize control values before synthesis so the waveform is driven by integer
    // phase/noise arithmetic. This keeps regenerated presentation audio stable and
    // avoids depending on a platform oscillator or external sample asset.
    const auto pitchMilliHz=static_cast<std::uint64_t>(std::max<long>(1L,std::lround(std::max(1.0f,cue.pitchHz)*1000.0f)));
    const auto pulseMilliHz=static_cast<std::uint64_t>(std::max<long>(0L,std::lround(std::max(0.0f,cue.pulseHz)*1000.0f)));
    const std::uint64_t phaseDen=static_cast<std::uint64_t>(out.sampleRate)*1000ULL;
    const std::uint32_t phaseStep=static_cast<std::uint32_t>((pitchMilliHz*(1ULL<<32U))/phaseDen);
    const std::uint32_t pulseStep=pulseMilliHz==0?0U:static_cast<std::uint32_t>((pulseMilliHz*(1ULL<<32U))/phaseDen);
    const int gainQ=std::clamp(static_cast<int>(std::lround(cue.gain*32767.0f)),0,32767);
    const int lowQ=std::clamp(static_cast<int>(std::lround(cue.lowFrequencyShare*32767.0f)),0,32767);

    int toneWeight=24576;
    int noiseWeight=8192;
    int harmonicWeight=4096;
    switch(cue.timbre) {
        case AudioTimbre::EarthThud: toneWeight=24576;noiseWeight=12288;harmonicWeight=2048;break;
        case AudioTimbre::StoneKnock: toneWeight=28672;noiseWeight=6144;harmonicWeight=6144;break;
        case AudioTimbre::WoodClack: toneWeight=26624;noiseWeight=8192;harmonicWeight=8192;break;
        case AudioTimbre::MetalRing: toneWeight=28672;noiseWeight=3072;harmonicWeight=12288;break;
        case AudioTimbre::GlassChime: toneWeight=30720;noiseWeight=1024;harmonicWeight=14336;break;
        case AudioTimbre::FluidSplash: toneWeight=12288;noiseWeight=24576;harmonicWeight=1024;break;
        case AudioTimbre::MachineHum: toneWeight=28672;noiseWeight=2048;harmonicWeight=4096;break;
        case AudioTimbre::OrganicRustle: toneWeight=8192;noiseWeight=26624;harmonicWeight=2048;break;
        case AudioTimbre::Wind: toneWeight=4096;noiseWeight=28672;harmonicWeight=1024;break;
        case AudioTimbre::AcidHiss: toneWeight=2048;noiseWeight=30720;harmonicWeight=4096;break;
        case AudioTimbre::RadiationTicks: toneWeight=18432;noiseWeight=12288;harmonicWeight=12288;break;
        case AudioTimbre::PressureCreak: toneWeight=28672;noiseWeight=6144;harmonicWeight=2048;break;
        case AudioTimbre::VacuumMuffle: toneWeight=24576;noiseWeight=3072;harmonicWeight=1024;break;
        case AudioTimbre::OrderlyRelay: toneWeight=28672;noiseWeight=1536;harmonicWeight=12288;break;
        case AudioTimbre::PrecisionServo: toneWeight=24576;noiseWeight=4096;harmonicWeight=14336;break;
        case AudioTimbre::FrontierRattle: toneWeight=14336;noiseWeight=22528;harmonicWeight=6144;break;
    }

    std::uint32_t phase=static_cast<std::uint32_t>(cue.fingerprint);
    std::uint32_t pulsePhase=static_cast<std::uint32_t>(cue.fingerprint>>32U);
    for(int i=0;i<sampleCount;++i) {
        phase+=phaseStep;
        pulsePhase+=pulseStep;
        const int saw=static_cast<int>(phase>>16U)-32768;
        const int triangle=32767-2*std::abs(saw);
        const std::uint32_t harmonicPhase=phase*2U+static_cast<std::uint32_t>(cue.variation*0x1f123bb5U);
        const int harmonic=32767-2*std::abs(static_cast<int>(harmonicPhase>>16U)-32768);
        const int noise=static_cast<int>((mix64(cue.fingerprint^static_cast<std::uint64_t>(i)*0x9E3779B97F4A7C15ULL)>>48U)&0xFFFFU)-32768;

        std::int64_t mixed=static_cast<std::int64_t>(triangle)*toneWeight+
                           static_cast<std::int64_t>(noise)*noiseWeight+
                           static_cast<std::int64_t>(harmonic)*harmonicWeight;
        mixed/=32768;

        // Low-frequency share softens high/noisy timbres; pulse provides the
        // instrumentation cadence used for radiation/relays without storing audio.
        mixed=(mixed*(32767-lowQ)+static_cast<std::int64_t>(triangle)*lowQ)/32767;
        if(pulseStep!=0U) {
            const int pulse=(pulsePhase&0x80000000U)?32767:19660;
            mixed=mixed*pulse/32767;
        }
        if(!cue.loop) {
            const int attack=std::max(1,sampleCount/40);
            const int release=std::max(1,sampleCount/3);
            int envelope=32767;
            if(i<attack) envelope=32767*i/attack;
            if(i>sampleCount-release) envelope=std::min(envelope,32767*(sampleCount-i)/release);
            mixed=mixed*std::max(0,envelope)/32767;
        }
        mixed=mixed*gainQ/32767;
        out.pcm[static_cast<std::size_t>(i)]=static_cast<std::int16_t>(std::clamp<std::int64_t>(mixed,-32767,32767));
    }

    std::uint64_t fp=1469598103934665603ULL;
    fp=fnvMix(fp,static_cast<std::uint64_t>(out.sampleRate));
    fp=fnvMix(fp,static_cast<std::uint64_t>(out.loop));
    for(const auto sample:out.pcm) fp=fnvMix(fp,static_cast<std::uint16_t>(sample));
    out.fingerprint=fp;
    return out;
}

void accumulate(PresentationTelemetry& telemetry, const GeneratedMaterialField& field) {
    ++telemetry.materialFields;
    telemetry.materialPixels+=static_cast<int>(field.albedo.size());
}

void accumulate(PresentationTelemetry& telemetry, const DetailClusterPacket& cluster) {
    ++telemetry.detailClusters;
    telemetry.detailInstances+=static_cast<int>(cluster.instances.size());
    telemetry.decorativeTriangles+=cluster.estimatedTriangles;
    telemetry.shadowCasters+=cluster.shadowCasters;
}

void accumulate(PresentationTelemetry& telemetry, const EffectPacket& effect) {
    ++telemetry.effectPackets;
    telemetry.particles+=static_cast<int>(effect.particles.size());
    telemetry.effectLights+=effect.dynamicLights;
    telemetry.effectWorkUnits+=effect.estimatedWorkUnits;
}

} // namespace elysium
