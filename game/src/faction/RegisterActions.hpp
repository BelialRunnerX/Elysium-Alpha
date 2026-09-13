#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace elysium::faction {

enum class RegisterBand : std::uint8_t { Noted, Marked, Hunted };
enum class RegisterPhase : std::uint8_t {
    Announced,
    Fighting,
    InterWave,
    Succeeded,
    Failed,
    Cancelled
};

enum class RegisterNoticeKind : std::uint8_t {
    Announcement,
    Rescheduled,
    WaveRequested,
    WaveCleared,
    Success,
    StandingRelief,
    RewardGranted,
    BeaconStrike,
    Cancelled
};

enum class RegisterFailure : std::uint8_t {
    None,
    InvalidPolicy,
    InvalidIdentity,
    InvalidSuspicion,
    NoEligiblePolicy,
    DuplicateAction,
    AlreadyActiveForPlayerSystem,
    RevisionConflict,
    InvalidPhase,
    ClockRegression,
    TickOverflow,
    InvalidSnapshot
};

struct RegisterWaveDefinition {
    std::uint64_t encounterTemplate{};
    std::uint32_t ordinal{};
    bool praetorStyle{};
};

struct RegisterPolicy {
    RegisterBand band{};
    double minimumSuspicion{};
    std::uint64_t preparationTicks{};
    std::uint64_t waveGapTicks{};
    std::vector<RegisterWaveDefinition> waves;
    std::uint64_t rewardContent{};
    double pressureRelief{};
    double successFloorHint{};
};

struct RegisterAction {
    std::uint64_t id{};
    std::uint64_t player{};
    std::uint64_t system{};
    std::uint64_t claim{};
    std::uint64_t beacon{};
    std::uint64_t seed{};
    std::uint64_t dueTick{};
    std::uint64_t revision{};
    RegisterBand band{};
    RegisterPhase phase{};
    std::uint32_t wave{};
    RegisterPolicy policy;
};

struct RegisterNotice {
    std::uint64_t sequence{};
    std::uint64_t action{};
    std::uint64_t revision{};
    std::uint64_t player{};
    std::uint64_t system{};
    std::uint64_t claim{};
    std::uint64_t target{};
    std::uint64_t content{};
    std::uint64_t deterministicSeed{};
    std::uint64_t dueTick{};
    RegisterNoticeKind kind{};
    double pressureRelief{};
    double floorHint{};
};

struct RegisterObservation {
    std::uint64_t action{};
    std::uint64_t revision{};
    std::uint64_t player{};
    std::uint64_t system{};
    std::uint64_t claim{};
    std::uint64_t beacon{};
    RegisterBand band{};
    RegisterPhase phase{};
    std::uint32_t currentWave{};
    std::uint32_t totalWaves{};
    std::uint64_t dueTick{};
    std::uint64_t remainingTicks{};
    std::uint64_t nextEncounterTemplate{};
    bool preparationWindow{};
    bool terminal{};
};

struct RegisterSnapshot {
    std::uint64_t clock{};
    std::uint64_t nextNoticeSequence{1};
    std::vector<RegisterPolicy> policies;
    std::vector<RegisterAction> actions;
    std::vector<RegisterNotice> outbox;
};

class RegisterActions {
public:
    bool publish(RegisterPolicy policy);
    bool replacePolicy(RegisterPolicy policy);

    bool schedule(RegisterAction action, double suspicionSnapshot, std::uint64_t now);
    bool reschedule(std::uint64_t id, std::uint64_t revision, std::uint64_t dueTick);
    bool cancel(std::uint64_t id, std::uint64_t revision);
    bool advance(std::uint64_t now);
    bool finishWave(std::uint64_t id, std::uint64_t revision, bool success, std::uint64_t now);

    [[nodiscard]] const RegisterAction* find(std::uint64_t id) const;
    [[nodiscard]] std::optional<RegisterObservation> observe(std::uint64_t id, std::uint64_t now) const;
    [[nodiscard]] std::vector<RegisterObservation> observeAll(std::uint64_t now) const;
    [[nodiscard]] RegisterSnapshot snapshot() const;
    bool restore(const RegisterSnapshot& snapshot);
    [[nodiscard]] std::vector<RegisterNotice> drain();

    [[nodiscard]] RegisterFailure lastFailure() const noexcept { return lastFailure_; }
    [[nodiscard]] std::uint64_t clock() const noexcept { return clock_; }

    [[nodiscard]] static std::optional<RegisterBand> bandForSuspicion(
        double suspicion,
        const std::map<RegisterBand, RegisterPolicy>& policies);

private:
    bool fail(RegisterFailure failure) noexcept;
    void emit(const RegisterAction& action,
              RegisterNoticeKind kind,
              std::uint64_t content = 0,
              double pressureRelief = 0.0,
              double floorHint = 0.0);
    [[nodiscard]] std::uint64_t waveSeed(const RegisterAction& action) const noexcept;

    std::map<RegisterBand, RegisterPolicy> policies_;
    std::map<std::uint64_t, RegisterAction> actions_;
    std::vector<RegisterNotice> notices_;
    std::uint64_t clock_{};
    std::uint64_t nextNoticeSequence_{1};
    RegisterFailure lastFailure_{RegisterFailure::None};
};

} // namespace elysium::faction
