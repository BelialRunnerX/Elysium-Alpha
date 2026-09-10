/** Design-doc Suspicion + Register Action (Part 24 Empire). */
export class SuspicionState {
  constructor() {
    this.value = 0;
    this.claimFloor = 0;
  }

  raise(amount) {
    this.value = Math.min(100, this.value + amount);
  }

  setClaimFloor(floor) {
    this.claimFloor = Math.max(0, Math.min(70, floor));
    if (this.value < this.claimFloor) this.value = this.claimFloor;
  }

  decay(dt, rate = 0.06) {
    if (this.value > this.claimFloor) {
      this.value = Math.max(this.claimFloor, this.value - rate * dt);
    }
  }

  band() {
    if (this.value >= 75) return 'HUNTED';
    if (this.value >= 45) return 'MARKED';
    if (this.value >= 20) return 'WATCHED';
    return 'CLEAR';
  }
}

export class RegisterActionDirector {
  constructor() {
    this.active = false;
    this.wave = 0;
    this.totalWaves = 0;
    this.timer = 0;
    this.cooldown = 0;
  }

  tick(dt, suspicion, claimed, spawnWave) {
    if (this.cooldown > 0) this.cooldown -= dt;
    if (!claimed) {
      this.active = false;
      return null;
    }
    const band = suspicion.band();
    if (!this.active && this.cooldown <= 0 && (band === 'MARKED' || band === 'HUNTED')) {
      this.active = true;
      this.wave = 0;
      this.totalWaves = band === 'HUNTED' ? 5 : 3;
      this.timer = 2;
      return { type: 'announce', waves: this.totalWaves, band };
    }
    if (this.active) {
      this.timer -= dt;
      if (this.timer <= 0) {
        this.wave += 1;
        spawnWave(this.wave, this.totalWaves);
        if (this.wave >= this.totalWaves) {
          this.active = false;
          this.cooldown = 90;
          return { type: 'complete', waves: this.totalWaves };
        }
        this.timer = 14;
        return { type: 'wave', wave: this.wave, total: this.totalWaves };
      }
    }
    return null;
  }
}
