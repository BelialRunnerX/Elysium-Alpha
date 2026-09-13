/** Deterministic value noise used by terrain + ore rolls. */
export function hash(x, z, seed) {
  let n = (x * 374761393 + z * 668265263) ^ seed;
  n = (n ^ (n >>> 13)) * 1274126177;
  return ((n ^ (n >>> 16)) >>> 0) / 4294967295;
}

export function lerp(a, b, t) { return a + (b - a) * t; }
export function clamp(v, a, b) { return Math.max(a, Math.min(b, v)); }
export function smooth(t) { return t * t * (3 - 2 * t); }

export function valueNoise(x, z, seed, scale) {
  const fx = x / scale;
  const fz = z / scale;
  const x0 = Math.floor(fx);
  const z0 = Math.floor(fz);
  const tx = smooth(fx - x0);
  const tz = smooth(fz - z0);
  const a = hash(x0, z0, seed);
  const b = hash(x0 + 1, z0, seed);
  const c = hash(x0, z0 + 1, seed);
  const d = hash(x0 + 1, z0 + 1, seed);
  return lerp(lerp(a, b, tx), lerp(c, d, tx), tz);
}

export function fbm(x, z, seed, octaves = 4) {
  let amp = 1;
  let freq = 1;
  let sum = 0;
  let norm = 0;
  for (let i = 0; i < octaves; i++) {
    sum += valueNoise(x * freq, z * freq, seed ^ (i * 97), 1) * amp;
    norm += amp;
    amp *= 0.5;
    freq *= 2;
  }
  return sum / norm;
}

export function heightAt(x, z, planet) {
  const s = planet.seed;
  let h =
    valueNoise(x, z, s, 52) * 16 +
    valueNoise(x, z, s ^ 0x91, 19) * 7 +
    valueNoise(x, z, s ^ 0x37, 8) * 2.4 +
    fbm(x * 0.02, z * 0.02, s ^ 0xaa, 3) * 3;
  if (planet.key === 'oceanic') h = h * 0.5 + 3.5;
  if (planet.key === 'barren') h = h * 0.72 + 2;
  if (planet.key === 'scorched') h *= 1.2;
  if (planet.key === 'frozen') h = h * 0.85 + 3;
  if (planet.key === 'anomalous') h += Math.sin(x * 0.07 + z * 0.05) * 3;
  return h;
}
