/** Vertical-slice config — Elysium design synthesis + Part 24. */
export const SAVE_KEY = 'elysium_play_v2';
export const WORLD = 220;
export const SEA = 8;

export const PLANETS = [
  {
    id: 0, key: 'temperate', name: 'TEMPERATE // FRONTIER PLAINS',
    blurb: 'Breathable homestead. Broad resources. Soft weather.',
    seed: 0x54454d50, sky: [0.42, 0.62, 0.78], fog: [0.58, 0.72, 0.76],
    ground: [0.25, 0.42, 0.22], rock: [0.38, 0.36, 0.32], accent: [0.35, 0.9, 0.6],
    oxygenDrain: 0, hazard: 0, weather: 'mist', slice: true,
  },
  {
    id: 1, key: 'barren', name: 'BARREN // REGOLITH PLAIN',
    blurb: 'Vacuum. Exposed geology. Suit-critical O₂.',
    seed: 0x42415252, sky: [0.08, 0.1, 0.16], fog: [0.12, 0.12, 0.16],
    ground: [0.48, 0.4, 0.34], rock: [0.3, 0.28, 0.32], accent: [0.9, 0.72, 0.4],
    oxygenDrain: 1.8, hazard: 0, weather: 'dust', slice: true,
  },
  {
    id: 2, key: 'scorched', name: 'SCORCHED // BASALT PLAIN',
    blurb: 'Thermal pressure. Magma seams. Hard mining.',
    seed: 0x53434f52, sky: [0.28, 0.12, 0.1], fog: [0.38, 0.18, 0.12],
    ground: [0.2, 0.12, 0.1], rock: [0.16, 0.14, 0.14], accent: [0.95, 0.4, 0.15],
    oxygenDrain: 0.55, hazard: 0.55, weather: 'ash', slice: true,
  },
  {
    id: 3, key: 'frozen', name: 'FROZEN // CRYOVOLCANIC FRONTIER',
    blurb: 'Cryogenic wind. Sparse ore under ice.',
    seed: 0x46524f5a, sky: [0.55, 0.68, 0.82], fog: [0.72, 0.8, 0.88],
    ground: [0.78, 0.86, 0.92], rock: [0.45, 0.52, 0.58], accent: [0.55, 0.85, 1],
    oxygenDrain: 0.35, hazard: 0.35, weather: 'snow', slice: false,
  },
  {
    id: 4, key: 'toxic', name: 'TOXIC // CAUSTIC BIOSPHERE',
    blurb: 'Corrosive haze. Lush but lethal.',
    seed: 0x544f5843, sky: [0.28, 0.42, 0.22], fog: [0.35, 0.5, 0.28],
    ground: [0.22, 0.36, 0.16], rock: [0.28, 0.26, 0.18], accent: [0.7, 1, 0.35],
    oxygenDrain: 1.1, hazard: 0.45, weather: 'spores', slice: false,
  },
  {
    id: 5, key: 'irradiated', name: 'IRRADIATED // RAD-STORM WASTE',
    blurb: 'Rad storms. Unstable soil. Imperial watch.',
    seed: 0x49525244, sky: [0.2, 0.26, 0.16], fog: [0.28, 0.34, 0.18],
    ground: [0.3, 0.34, 0.18], rock: [0.22, 0.24, 0.18], accent: [0.85, 1, 0.25],
    oxygenDrain: 0.7, hazard: 0.7, weather: 'ash', slice: false,
  },
  {
    id: 6, key: 'oceanic', name: 'OCEANIC // PELAGIC WORLD',
    blurb: 'Shallow shelves and deep pressure.',
    seed: 0x4f434541, sky: [0.22, 0.42, 0.62], fog: [0.28, 0.48, 0.6],
    ground: [0.52, 0.46, 0.3], rock: [0.34, 0.38, 0.4], accent: [0.35, 0.75, 0.95],
    oxygenDrain: 0.2, hazard: 0.15, weather: 'mist', slice: false,
  },
  {
    id: 7, key: 'anomalous', name: 'ANOMALOUS // IMPERIAL EXCLUSION',
    blurb: 'Nullspire bleed. Claim denied by doctrine.',
    seed: 0x414e4f4d, sky: [0.1, 0.06, 0.16], fog: [0.16, 0.08, 0.2],
    ground: [0.18, 0.1, 0.26], rock: [0.12, 0.08, 0.18], accent: [0.55, 0.95, 0.85],
    oxygenDrain: 0.9, hazard: 0.8, weather: 'spores', slice: false,
  },
];

export const BLOCKS = [
  { id: 0, name: 'DIRT', color: 0x6b4f3a, placeable: true },
  { id: 1, name: 'STONE', color: 0x6e7270, placeable: true },
  { id: 2, name: 'PLANKS', color: 0x8b6a3f, placeable: true },
  { id: 3, name: 'STEEL', color: 0x8ea0a8, placeable: true },
  { id: 4, name: 'BEACON', color: 0x4affc0, placeable: true, machine: 'beacon' },
  { id: 5, name: 'COPPER', color: 0xb87333, placeable: false },
  { id: 6, name: 'TIN', color: 0xa0a8b0, placeable: false },
  { id: 7, name: 'IRON', color: 0x7a8490, placeable: false },
  { id: 8, name: 'COAL', color: 0x2a2a2a, placeable: false },
  { id: 9, name: 'BRONZE', color: 0xcd7f32, placeable: false },
  { id: 10, name: 'ATMO', color: 0x5ec8ff, placeable: true, machine: 'atmosphere' },
  { id: 11, name: 'BURNER', color: 0xe07a5f, placeable: true, machine: 'burner' },
];

export const RECIPES = [
  { id: 'bronze_head', name: 'Bronze Mining Head', needs: { 5: 3, 6: 1 }, gives: { tool: 2 }, key: '1' },
  { id: 'steel_head', name: 'Steel Mining Head', needs: { 7: 4, 8: 2 }, gives: { tool: 3 }, key: '2' },
  { id: 'steel_plate', name: 'Steel Plate ×2', needs: { 7: 2, 8: 1 }, gives: { 3: 2 }, key: '3' },
  { id: 'bronze_bar', name: 'Bronze Ingot ×2', needs: { 5: 2, 6: 1 }, gives: { 9: 2 }, key: '4' },
  { id: 'beacon', name: 'Registry Beacon', needs: { 3: 4, 5: 2 }, gives: { 4: 1 }, key: '5' },
  { id: 'burner', name: 'Burner Generator', needs: { 3: 2, 5: 2 }, gives: { 11: 1 }, key: '6' },
  { id: 'atmo', name: 'Atmosphere Unit', needs: { 3: 3, 5: 2 }, gives: { 10: 1 }, key: '7' },
  { id: 'planks', name: 'Planks ×4', needs: { 0: 2 }, gives: { 2: 4 }, key: '8' },
];

export const TOOL_TIERS = [
  { tier: 1, name: 'Improvised', mineSpeed: 1, canMine: [0, 1, 5, 8] },
  { tier: 2, name: 'Bronze', mineSpeed: 1.6, canMine: [0, 1, 5, 6, 7, 8] },
  { tier: 3, name: 'Steel', mineSpeed: 2.2, canMine: [0, 1, 5, 6, 7, 8] },
];
