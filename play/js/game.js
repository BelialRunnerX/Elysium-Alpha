import * as THREE from 'three';
import { PointerLockControls } from 'three/addons/controls/PointerLockControls.js';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { OutputPass } from 'three/addons/postprocessing/OutputPass.js';
import { RoomEnvironment } from 'three/addons/environments/RoomEnvironment.js';
import { bindMaterials } from './materials.js';
import {
  createTerraDrone,
  createRover,
  createOrbiScout,
  createHoverBike,
  createFieldFabricator,
} from './assets/heroes.js';

bindMaterials(THREE);

const SAVE_KEY = 'elysium_browser_save_v1';
const WORLD = 192;
const SEA = 9;
const VIEW = 90;

const PLANETS = [
  {
    id: 0,
    key: 'temperate',
    name: 'TEMPERATE // FRONTIER PLAINS',
    blurb: 'Breathable homestead. Broad resources. Soft weather.',
    seed: 0x54454d50,
    sky: [0.45, 0.66, 0.8],
    fog: [0.62, 0.74, 0.78],
    ground: [0.28, 0.42, 0.24],
    rock: [0.35, 0.34, 0.32],
    accent: [0.29, 0.84, 0.55],
    oxygenDrain: 0,
    hazard: 0,
    weather: 'dust',
  },
  {
    id: 1,
    key: 'barren',
    name: 'BARREN // REGOLITH PLAIN',
    blurb: 'Vacuum. Exposed geology. Suit-critical.',
    seed: 0x42415252,
    sky: [0.13, 0.16, 0.23],
    fog: [0.18, 0.17, 0.2],
    ground: [0.45, 0.38, 0.32],
    rock: [0.28, 0.27, 0.3],
    accent: [0.85, 0.7, 0.45],
    oxygenDrain: 1.6,
    hazard: 0,
    weather: 'dust',
  },
  {
    id: 2,
    key: 'scorched',
    name: 'SCORCHED // BASALT PLAIN',
    blurb: 'Thermal pressure. Magma seams. Hard mining.',
    seed: 0x53434f52,
    sky: [0.31, 0.15, 0.15],
    fog: [0.4, 0.2, 0.16],
    ground: [0.22, 0.14, 0.12],
    rock: [0.18, 0.16, 0.16],
    accent: [0.95, 0.45, 0.2],
    oxygenDrain: 0.55,
    hazard: 0.55,
    weather: 'ash',
  },
  {
    id: 3,
    key: 'frozen',
    name: 'FROZEN // CRYOVOLCANIC FRONTIER',
    blurb: 'Cryogenic wind. Sparse ore under ice.',
    seed: 0x46524f5a,
    sky: [0.55, 0.68, 0.82],
    fog: [0.72, 0.8, 0.88],
    ground: [0.78, 0.86, 0.92],
    rock: [0.45, 0.52, 0.58],
    accent: [0.55, 0.85, 1.0],
    oxygenDrain: 0.35,
    hazard: 0.35,
    weather: 'snow',
  },
  {
    id: 4,
    key: 'toxic',
    name: 'TOXIC // CAUSTIC BIOSPHERE',
    blurb: 'Corrosive haze. Lush but lethal.',
    seed: 0x544f5843,
    sky: [0.28, 0.42, 0.22],
    fog: [0.35, 0.5, 0.28],
    ground: [0.25, 0.38, 0.18],
    rock: [0.3, 0.28, 0.2],
    accent: [0.7, 1.0, 0.35],
    oxygenDrain: 1.1,
    hazard: 0.45,
    weather: 'spores',
  },
  {
    id: 5,
    key: 'irradiated',
    name: 'IRRADIATED // RAD-STORM WASTE',
    blurb: 'Rad storms. Unstable soil. Imperial watch.',
    seed: 0x49525244,
    sky: [0.22, 0.28, 0.18],
    fog: [0.3, 0.36, 0.2],
    ground: [0.32, 0.36, 0.2],
    rock: [0.24, 0.26, 0.2],
    accent: [0.85, 1.0, 0.25],
    oxygenDrain: 0.7,
    hazard: 0.7,
    weather: 'ash',
  },
  {
    id: 6,
    key: 'oceanic',
    name: 'OCEANIC // PELAGIC WORLD',
    blurb: 'Shallow shelves and deep pressure.',
    seed: 0x4f434541,
    sky: [0.25, 0.45, 0.65],
    fog: [0.3, 0.5, 0.62],
    ground: [0.55, 0.48, 0.32],
    rock: [0.35, 0.4, 0.42],
    accent: [0.35, 0.75, 0.95],
    oxygenDrain: 0.2,
    hazard: 0.15,
    weather: 'mist',
  },
  {
    id: 7,
    key: 'anomalous',
    name: 'ANOMALOUS // IMPERIAL EXCLUSION',
    blurb: 'Nullspire bleed. Claim denied by doctrine.',
    seed: 0x414e4f4d,
    sky: [0.12, 0.08, 0.18],
    fog: [0.18, 0.1, 0.22],
    ground: [0.2, 0.12, 0.28],
    rock: [0.14, 0.1, 0.2],
    accent: [0.55, 0.95, 0.85],
    oxygenDrain: 0.9,
    hazard: 0.8,
    weather: 'spores',
  },
];

const BLOCKS = [
  { id: 0, name: 'DIRT', color: 0x6b4f3a },
  { id: 1, name: 'STONE', color: 0x6e7270 },
  { id: 2, name: 'PLANKS', color: 0x8b6a3f },
  { id: 3, name: 'STEEL', color: 0x8ea0a8 },
  { id: 4, name: 'BEACON', color: 0x4affc0 },
  { id: 5, name: 'COPPER', color: 0xb87333 },
  { id: 6, name: 'IRON', color: 0x7a8490 },
  { id: 7, name: 'COAL', color: 0x2a2a2a },
];

const $ = (id) => document.getElementById(id);
const clamp = (v, a, b) => Math.max(a, Math.min(b, v));
const lerp = (a, b, t) => a + (b - a) * t;
const hash = (x, z, seed) => {
  let n = (x * 374761393 + z * 668265263) ^ seed;
  n = (n ^ (n >>> 13)) * 1274126177;
  return ((n ^ (n >>> 16)) >>> 0) / 4294967295;
};
const smooth = (t) => t * t * (3 - 2 * t);
const valueNoise = (x, z, seed, scale) => {
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
};

function heightAt(x, z, planet) {
  const s = planet.seed;
  let h =
    valueNoise(x, z, s, 48) * 14 +
    valueNoise(x, z, s ^ 0x91, 18) * 6 +
    valueNoise(x, z, s ^ 0x37, 7) * 2.2;
  if (planet.key === 'oceanic') h = h * 0.55 + 4;
  if (planet.key === 'barren') h = h * 0.7 + 2;
  if (planet.key === 'scorched') h = h * 1.15;
  if (planet.key === 'frozen') h = h * 0.85 + 3;
  if (planet.key === 'anomalous') h += Math.sin(x * 0.08 + z * 0.05) * 2.5;
  return h;
}

const state = {
  mode: 'title',
  planetIndex: 0,
  claimed: Array(PLANETS.length).fill(false),
  suspicion: 0,
  selected: 0,
  inventory: Object.fromEntries(BLOCKS.map((b) => [b.id, b.id === 0 ? 24 : b.id < 4 ? 12 : 4])),
  vit: 100,
  o2: 100,
  eng: 100,
  hng: 100,
  messageTimer: 0,
  message: '',
  worldTime: 0.28,
  edits: {},
  enemies: [],
  positions: PLANETS.map(() => null),
};

const renderer = new THREE.WebGLRenderer({
  canvas: $('c'),
  antialias: true,
  powerPreference: 'high-performance',
});
renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
renderer.setSize(innerWidth, innerHeight);
renderer.outputColorSpace = THREE.SRGBColorSpace;
renderer.toneMapping = THREE.ACESFilmicToneMapping;
renderer.toneMappingExposure = 1.05;
renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;

const scene = new THREE.Scene();
const pmrem = new THREE.PMREMGenerator(renderer);
scene.environment = pmrem.fromScene(new RoomEnvironment(), 0.04).texture;
scene.environmentIntensity = 0.4;
const camera = new THREE.PerspectiveCamera(72, innerWidth / innerHeight, 0.08, 1200);
const controls = new PointerLockControls(camera, document.body);

const hemi = new THREE.HemisphereLight(0xd8e8ff, 0x3a3228, 0.75);
scene.add(hemi);
const sun = new THREE.DirectionalLight(0xfff5e0, 1.85);
sun.castShadow = true;
sun.shadow.mapSize.set(2048, 2048);
sun.shadow.camera.near = 1;
sun.shadow.camera.far = 220;
sun.shadow.camera.left = -80;
sun.shadow.camera.right = 80;
sun.shadow.camera.top = 80;
sun.shadow.camera.bottom = -80;
scene.add(sun);
scene.add(sun.target);

const ambientFill = new THREE.AmbientLight(0x405060, 0.32);
const rimLight = new THREE.DirectionalLight(0x88e8ff, 0.35);
rimLight.position.set(-40, 30, -60);
scene.add(rimLight);
scene.add(ambientFill);

const skyMat = new THREE.ShaderMaterial({
  side: THREE.BackSide,
  depthWrite: false,
  uniforms: {
    topColor: { value: new THREE.Color(0x74a9ce) },
    midColor: { value: new THREE.Color(0xc2d4c8) },
    bottomColor: { value: new THREE.Color(0x1a2428) },
    sunDir: { value: new THREE.Vector3(0.4, 0.8, 0.2) },
    sunColor: { value: new THREE.Color(0xffe7b0) },
    timeOfDay: { value: 0.25 },
  },
  vertexShader: `
    varying vec3 vWorld;
    void main() {
      vec4 p = modelMatrix * vec4(position, 1.0);
      vWorld = normalize(p.xyz);
      gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
  `,
  fragmentShader: `
    uniform vec3 topColor;
    uniform vec3 midColor;
    uniform vec3 bottomColor;
    uniform vec3 sunDir;
    uniform vec3 sunColor;
    uniform float timeOfDay;
    varying vec3 vWorld;
    void main() {
      float h = clamp(vWorld.y * 0.5 + 0.5, 0.0, 1.0);
      vec3 col = mix(bottomColor, midColor, smoothstep(0.0, 0.45, h));
      col = mix(col, topColor, smoothstep(0.35, 1.0, h));
      float sunAmt = pow(max(dot(normalize(vWorld), normalize(sunDir)), 0.0), 32.0);
      col += sunColor * sunAmt * (0.55 + 0.45 * timeOfDay);
      float night = smoothstep(0.18, 0.0, timeOfDay);
      col = mix(col, col * 0.18 + vec3(0.02, 0.04, 0.08), night);
      gl_FragColor = vec4(col, 1.0);
    }
  `,
});
const sky = new THREE.Mesh(new THREE.SphereGeometry(800, 48, 32), skyMat);
scene.add(sky);

const sunMesh = new THREE.Mesh(
  new THREE.SphereGeometry(8, 24, 24),
  new THREE.MeshBasicMaterial({ color: 0xffe4a8, toneMapped: false }),
);
scene.add(sunMesh);

let terrain = null;
let water = null;
const editGroup = new THREE.Group();
scene.add(editGroup);
const enemyGroup = new THREE.Group();
scene.add(enemyGroup);
const heroGroup = new THREE.Group();
scene.add(heroGroup);
const heroActors = [];
const particleGeo = new THREE.BufferGeometry();
const PARTICLE_COUNT = 900;
const particlePos = new Float32Array(PARTICLE_COUNT * 3);
for (let i = 0; i < PARTICLE_COUNT; i++) {
  particlePos[i * 3] = (Math.random() - 0.5) * 120;
  particlePos[i * 3 + 1] = Math.random() * 40;
  particlePos[i * 3 + 2] = (Math.random() - 0.5) * 120;
}
particleGeo.setAttribute('position', new THREE.BufferAttribute(particlePos, 3));
const particles = new THREE.Points(
  particleGeo,
  new THREE.PointsMaterial({
    color: 0xd7e2dc,
    size: 0.12,
    transparent: true,
    opacity: 0.55,
    depthWrite: false,
  }),
);
scene.add(particles);

const composer = new EffectComposer(renderer);
composer.addPass(new RenderPass(scene, camera));
const bloomPass = new UnrealBloomPass(new THREE.Vector2(innerWidth, innerHeight), 0.55, 0.5, 0.78);
composer.addPass(bloomPass);
composer.addPass(new OutputPass());

const velocity = new THREE.Vector3();
const direction = new THREE.Vector3();
const keys = { forward: false, back: false, left: false, right: false, sprint: false, jump: false };
let canJump = false;
let grounded = false;

function planet() {
  return PLANETS[state.planetIndex];
}

function editKey(x, y, z) {
  return `${state.planetIndex}:${x},${y},${z}`;
}

function surfaceY(x, z) {
  const ix = Math.floor(x);
  const iz = Math.floor(z);
  let y = Math.floor(heightAt(ix, iz, planet()));
  while (state.edits[editKey(ix, y + 1, iz)]) y += 1;
  while (y > -8 && state.edits[editKey(ix, y, iz)] === -1) y -= 1;
  return y;
}


function clearHeroes() {
  while (heroGroup.children.length) {
    const m = heroGroup.children.pop();
    m.traverse((o) => {
      if (o.geometry) o.geometry.dispose();
      // Do not dispose materials — elysiumMaterials() kit is shared/cached.
    });
  }
  heroActors.length = 0;
}

function placeHero(factory, x, z, yaw = 0, scale = 1) {
  const root = factory(THREE);
  const y = surfaceY(x, z) + 0.05;
  root.position.set(x, y, z);
  root.rotation.y = yaw;
  root.scale.setScalar(scale);
  root.userData.baseY = y + (root.userData.hover ? 1.15 : 0);
  root.position.y = root.userData.baseY;
  root.userData.t = Math.random() * Math.PI * 2;
  root.traverse((o) => {
    if (o.isMesh && o.userData.pulse && o.material && o.material.clone) {
      // keep existing pulse clones
    }
  });
  heroGroup.add(root);
  heroActors.push(root);
  return root;
}

function spawnHeroes() {
  clearHeroes();
  // Ceramic LZ pad — stages the hero camp like a concept sheet floor
  const padY = surfaceY(2, 0);
  const pad = new THREE.Mesh(
    new THREE.CylinderGeometry(9.5, 10, 0.18, 48),
    new THREE.MeshStandardMaterial({
      color: 0xe8eef2,
      roughness: 0.42,
      metalness: 0.08,
    }),
  );
  pad.position.set(2, padY + 0.05, 0);
  pad.receiveShadow = true;
  pad.userData.isPad = true;
  heroGroup.add(pad);
  const ring = new THREE.Mesh(
    new THREE.TorusGeometry(9.2, 0.06, 8, 64),
    new THREE.MeshStandardMaterial({
      color: 0x3de0ff,
      emissive: 0x3de0ff,
      emissiveIntensity: 1.8,
      roughness: 0.3,
      metalness: 0.2,
      toneMapped: false,
    }),
  );
  ring.rotation.x = Math.PI / 2;
  ring.position.set(2, padY + 0.16, 0);
  heroGroup.add(ring);
  const goldRing = new THREE.Mesh(
    new THREE.TorusGeometry(8.4, 0.04, 8, 64),
    new THREE.MeshStandardMaterial({ color: 0xe0b93a, roughness: 0.25, metalness: 1 }),
  );
  goldRing.rotation.x = Math.PI / 2;
  goldRing.position.set(2, padY + 0.15, 0);
  heroGroup.add(goldRing);

  // Field camp clustered at LZ so the art bar is unmistakable on Descend
  const roster = [
    [createTerraDrone, 4.5, -2.5, 0.5, 1.05],
    [createRover, -5.5, 3.5, -0.7, 1.15],
    [createOrbiScout, 2.5, 4.5, 0.3, 1.0],
    [createHoverBike, -3.5, -5.5, 1.2, 1.05],
    [createFieldFabricator, 7.5, 1.5, -0.4, 1.0],
    [createOrbiScout, -1.5, 1.2, 1.5, 0.9],
  ];
  for (const [factory, x, z, yaw, scale] of roster) {
    try {
      placeHero(factory, x, z, yaw, scale);
    } catch (err) {
      console.error('Hero spawn failed', factory.name, err);
      toast(`HERO BUILD FAULT · ${factory.name}`, 4);
    }
  }
}

function updateHeroes(dt) {
  for (const root of heroActors) {
    root.userData.t += dt;
    const t = root.userData.t;
    if (root.userData.hover) {
      root.position.y = root.userData.baseY + 0.35 + Math.sin(t * 1.7) * 0.12;
      root.rotation.y += dt * 0.35;
    } else {
      root.rotation.y += Math.sin(t * 0.6) * dt * 0.05;
    }
    root.traverse((o) => {
      // Only pulse cloned materials / lights — never shared kit materials
      if (o.isMesh && o.userData.pulse && o.material && o.material.emissiveIntensity != null && o.material !== o.userData._sharedEmissive) {
        o.material.emissiveIntensity = 1.6 + Math.sin(t * 3.2) * 0.7;
      }
      if (o.isLight && o.isPointLight) {
        o.intensity = 0.7 + Math.sin(t * 2.4) * 0.35;
      }
    });
  }
}

function nearestHero(maxDist = 7.5) {
  const p = controls.object.position;
  let best = null;
  let bestD = maxDist;
  for (const root of heroActors) {
    const d = root.position.distanceTo(p);
    if (d < bestD) {
      bestD = d;
      best = root;
    }
  }
  return best;
}

function buildTerrain() {
  $('loading').classList.remove('hidden');
  if (terrain) {
    scene.remove(terrain);
    terrain.geometry.dispose();
    terrain.material.dispose();
  }
  if (water) {
    scene.remove(water);
    water.geometry.dispose();
    water.material.dispose();
  }

  const p = planet();
  const seg = 160;
  const geo = new THREE.PlaneGeometry(WORLD, WORLD, seg, seg);
  geo.rotateX(-Math.PI / 2);
  const pos = geo.attributes.position;
  const colors = new Float32Array(pos.count * 3);
  const g = new THREE.Color(...p.ground);
  const r = new THREE.Color(...p.rock);
  const accent = new THREE.Color(...p.accent);

  for (let i = 0; i < pos.count; i++) {
    const x = pos.getX(i);
    const z = pos.getZ(i);
    const h = heightAt(x, z, p);
    pos.setY(i, h);
    const slope =
      Math.abs(heightAt(x + 1, z, p) - heightAt(x - 1, z, p)) +
      Math.abs(heightAt(x, z + 1, p) - heightAt(x, z - 1, p));
    const c = g.clone().lerp(r, clamp(slope * 0.18, 0, 1));
    if (p.key === 'temperate' && h > SEA + 1.5 && slope < 1.2) c.lerp(accent, 0.18);
    if (p.key === 'toxic') c.lerp(accent, 0.12 + hash(Math.floor(x), Math.floor(z), p.seed) * 0.2);
    if (p.key === 'scorched' && h < SEA - 1) c.setRGB(0.85, 0.25, 0.08);
    colors[i * 3] = c.r;
    colors[i * 3 + 1] = c.g;
    colors[i * 3 + 2] = c.b;
  }
  geo.setAttribute('color', new THREE.BufferAttribute(colors, 3));
  geo.computeVertexNormals();

  terrain = new THREE.Mesh(
    geo,
    new THREE.MeshStandardMaterial({
      vertexColors: true,
      roughness: 0.92,
      metalness: 0.04,
      flatShading: false,
    }),
  );
  terrain.receiveShadow = true;
  terrain.castShadow = true;
  scene.add(terrain);

  if (p.key === 'oceanic' || p.key === 'temperate' || p.key === 'frozen') {
    water = new THREE.Mesh(
      new THREE.PlaneGeometry(WORLD * 1.2, WORLD * 1.2, 1, 1),
      new THREE.MeshStandardMaterial({
        color: p.key === 'frozen' ? 0xa9c8d8 : 0x1f5f78,
        transparent: true,
        opacity: 0.62,
        roughness: 0.15,
        metalness: 0.35,
      }),
    );
    water.rotation.x = -Math.PI / 2;
    water.position.y = SEA - (p.key === 'oceanic' ? 0.2 : 2.5);
    scene.add(water);
  }

  scene.fog = new THREE.FogExp2(new THREE.Color(...p.fog), 0.012);
  skyMat.uniforms.topColor.value.setRGB(p.sky[0], p.sky[1] + 0.05, p.sky[2] + 0.08);
  skyMat.uniforms.midColor.value.setRGB(p.fog[0], p.fog[1], p.fog[2]);
  skyMat.uniforms.bottomColor.value.setRGB(p.sky[0] * 0.25, p.sky[1] * 0.25, p.sky[2] * 0.3);
  particles.material.color.setRGB(...p.accent);

  rebuildEdits();
  spawnEnemies();
  if (state.mode === 'play') spawnHeroes();
  else clearHeroes();
  $('planet-label').textContent = p.name;
  requestAnimationFrame(() => $('loading').classList.add('hidden'));
}

function rebuildEdits() {
  while (editGroup.children.length) {
    const m = editGroup.children.pop();
    m.geometry.dispose();
    m.material.dispose();
  }
  const geo = new THREE.BoxGeometry(1, 1, 1);
  for (const [key, id] of Object.entries(state.edits)) {
    if (!key.startsWith(`${state.planetIndex}:`) || id < 0) continue;
    const [, coords] = key.split(':');
    const [x, y, z] = coords.split(',').map(Number);
    const mat = new THREE.MeshStandardMaterial({
      color: BLOCKS[id]?.color ?? 0x888888,
      roughness: 0.75,
      metalness: id === 3 ? 0.55 : 0.05,
      emissive: id === 4 ? 0x145c45 : 0x000000,
      emissiveIntensity: id === 4 ? 0.7 : 0,
    });
    const mesh = new THREE.Mesh(geo, mat);
    mesh.position.set(x + 0.5, y + 0.5, z + 0.5);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    editGroup.add(mesh);
  }
}

function spawnEnemies() {
  while (enemyGroup.children.length) {
    const m = enemyGroup.children.pop();
    m.traverse((o) => {
      if (o.geometry) o.geometry.dispose();
      if (o.material) o.material.dispose();
    });
  }
  state.enemies = [];
  const p = planet();
  const count = 8 + Math.floor(state.suspicion / 12);
  for (let i = 0; i < count; i++) {
    const ang = (i / count) * Math.PI * 2 + hash(i, 3, p.seed);
    const dist = 28 + hash(i, 9, p.seed) * 40;
    const x = Math.cos(ang) * dist;
    const z = Math.sin(ang) * dist;
    const y = surfaceY(x, z) + 1.1;
    const root = new THREE.Group();
    const body = new THREE.Mesh(
      new THREE.CapsuleGeometry(0.35, 0.7, 4, 8),
      new THREE.MeshStandardMaterial({ color: 0x1c221f, roughness: 0.55, metalness: 0.35 }),
    );
    const eye = new THREE.Mesh(
      new THREE.SphereGeometry(0.14, 12, 12),
      new THREE.MeshStandardMaterial({
        color: 0x4affc0,
        emissive: 0x4affc0,
        emissiveIntensity: 1.4,
      }),
    );
    eye.position.set(0, 0.55, 0.22);
    body.castShadow = true;
    root.add(body, eye);
    root.position.set(x, y, z);
    enemyGroup.add(root);
    state.enemies.push({
      mesh: root,
      hp: 40 + i * 4,
      speed: 3.2 + hash(i, 1, p.seed) * 1.8,
      phase: Math.random() * Math.PI * 2,
    });
  }
}

function spawnPlayer(reset = false) {
  let pos = state.positions[state.planetIndex];
  if (!pos || reset) {
    const y = surfaceY(0, 0) + 2.2;
    pos = { x: 2, y, z: 2 };
    state.positions[state.planetIndex] = pos;
  }
  controls.object.position.set(pos.x, pos.y, pos.z);
  velocity.set(0, 0, 0);
}

function toast(msg, seconds = 2.4) {
  state.message = msg;
  state.messageTimer = seconds;
  const el = $('toast');
  el.textContent = msg;
  el.classList.add('show');
}

function setMode(mode) {
  state.mode = mode;
  $('title').classList.toggle('active', mode === 'title');
  $('title').classList.toggle('hidden', mode !== 'title');
  $('hud').classList.toggle('hidden', mode !== 'play');
  $('pause').classList.toggle('hidden', mode !== 'pause');
  $('pause').classList.toggle('active', mode === 'pause');
  $('travel').classList.toggle('hidden', mode !== 'travel');
  $('travel').classList.toggle('active', mode === 'travel');
  $('map').classList.toggle('hidden', mode !== 'map');
  $('map').classList.toggle('active', mode === 'map');
  if ($('craft')) {
    $('craft').classList.toggle('hidden', mode !== 'craft');
    $('craft').classList.toggle('active', mode === 'craft');
  }
  if (mode !== 'play' && controls.isLocked) {
    controls.unlock();
  }
  $("c").style.pointerEvents = mode === "play" ? "auto" : "none";
}

function persist() {
  const pos = controls.object.position;
  state.positions[state.planetIndex] = { x: pos.x, y: pos.y, z: pos.z };
  localStorage.setItem(
    SAVE_KEY,
    JSON.stringify({
      planetIndex: state.planetIndex,
      claimed: state.claimed,
      suspicion: state.suspicion,
      selected: state.selected,
      inventory: state.inventory,
      vit: state.vit,
      o2: state.o2,
      eng: state.eng,
      hng: state.hng,
      edits: state.edits,
      positions: state.positions,
      worldTime: state.worldTime,
    }),
  );
}

function loadSave() {
  try {
    const raw = localStorage.getItem(SAVE_KEY);
    if (!raw) return false;
    const data = JSON.parse(raw);
    Object.assign(state, {
      planetIndex: data.planetIndex ?? 0,
      claimed: data.claimed ?? state.claimed,
      suspicion: data.suspicion ?? 0,
      selected: data.selected ?? 0,
      inventory: data.inventory ?? state.inventory,
      vit: data.vit ?? 100,
      o2: data.o2 ?? 100,
      eng: data.eng ?? 100,
      hng: data.hng ?? 100,
      edits: data.edits ?? {},
      positions: data.positions ?? state.positions,
      worldTime: data.worldTime ?? 0.22,
    });
    return true;
  } catch {
    return false;
  }
}

function renderHotbar() {
  const bar = $('hotbar');
  bar.innerHTML = '';
  BLOCKS.forEach((b, i) => {
    const el = document.createElement('div');
    el.className = `slot${i === state.selected ? ' active' : ''}`;
    el.innerHTML = `<span class="n">${i + 1}</span>${b.name}<br>${state.inventory[b.id] ?? 0}`;
    bar.appendChild(el);
  });
}


function renderCraft() {
  const list = $('craft-list');
  if (!list) return;
  list.innerHTML = '';
  const recipes = [
    { name: 'Steel Plate ×2', needs: '2 IRON + 1 COAL', give: () => { state.inventory[3] = (state.inventory[3] ?? 0) + 2; } },
    { name: 'Registry Beacon', needs: '4 STEEL + 2 COPPER', give: () => { state.inventory[4] = (state.inventory[4] ?? 0) + 1; } },
    { name: 'Field Ration Pack', needs: '2 DIRT + 1 PLANKS', give: () => { state.vit = clamp(state.vit + 20, 0, 100); state.hng = clamp(state.hng + 25, 0, 100); } },
  ];
  for (const r of recipes) {
    const row = document.createElement('button');
    row.type = 'button';
    row.className = 'planet-card';
    row.innerHTML = `<h3>${r.name}</h3><p>${r.needs}</p>`;
    row.onclick = () => {
      r.give();
      renderHotbar();
      updateHud();
      toast(`FABRICATED · ${r.name}`, 2);
      raiseSuspicion(0.8);
    };
    list.appendChild(row);
  }
}

function renderTravel() {
  const list = $('planet-list');
  list.innerHTML = '';
  PLANETS.forEach((p, i) => {
    const card = document.createElement('button');
    card.type = 'button';
    card.className = `planet-card${i === state.planetIndex ? ' current' : ''}`;
    card.innerHTML = `<h3>${p.name}</h3><p>${p.blurb}</p>`;
    card.onclick = () => travelTo(i);
    list.appendChild(card);
  });
}

function renderMap() {
  const body = $('map-body');
  body.innerHTML = '';
  PLANETS.forEach((p, i) => {
    const row = document.createElement('div');
    row.className = 'map-row';
    const edits = Object.keys(state.edits).filter((k) => k.startsWith(`${i}:`)).length;
    row.innerHTML = `<strong>${i + 1}. ${p.name}</strong><span>edits ${edits} · claim ${
      state.claimed[i] ? 'FILED' : 'UNFILED'
    } · ${i === state.planetIndex ? 'YOU ARE HERE' : 'remote'}</span>`;
    body.appendChild(row);
  });
}

function travelTo(index) {
  persist();
  state.planetIndex = index;
  buildTerrain();
  spawnPlayer(false);
  setMode('play');
  toast(`ARRIVAL // ${planet().name}`, 3);
  updateHud();
}

function raiseSuspicion(amount) {
  state.suspicion = clamp(state.suspicion + amount, 0, 100);
  if (!state.claimed[state.planetIndex] && state.suspicion > 35) {
    toast('TERRITORIAL ATTENTION RISING — FILE A BEACON CLAIM', 3.2);
  }
}

function tryMine() {
  const raycaster = new THREE.Raycaster();
  raycaster.setFromCamera(new THREE.Vector2(0, 0), camera);
  raycaster.far = 6;
  const hits = raycaster.intersectObject(terrain);
  if (!hits.length) return;
  const hit = hits[0];
  const x = Math.floor(hit.point.x);
  const z = Math.floor(hit.point.z);
  const y = Math.floor(hit.point.y);
  const key = editKey(x, y, z);
  if (state.edits[key] === -1) return;
  const roll = hash(x, z, planet().seed ^ y);
  let drop = 1;
  if (roll > 0.92) drop = 5;
  else if (roll > 0.84) drop = 6;
  else if (roll > 0.74) drop = 7;
  else if (roll > 0.55) drop = 0;
  else drop = 1;
  state.edits[key] = -1;
  state.inventory[drop] = (state.inventory[drop] ?? 0) + 1;
  state.eng = clamp(state.eng - 1.5, 0, 100);
  raiseSuspicion(0.35);
  rebuildEdits();
  renderHotbar();
  toast(`EXTRACTED ${BLOCKS[drop].name}`, 1.2);
}

function tryPlace() {
  const id = state.selected;
  if ((state.inventory[id] ?? 0) <= 0) {
    toast('NO STOCK', 1.2);
    return;
  }
  const raycaster = new THREE.Raycaster();
  raycaster.setFromCamera(new THREE.Vector2(0, 0), camera);
  raycaster.far = 6;
  const hits = raycaster.intersectObject(terrain);
  if (!hits.length) return;
  const hit = hits[0];
  const n = hit.face.normal.clone().transformDirection(terrain.matrixWorld).normalize();
  const p = hit.point.clone().addScaledVector(n, 0.51);
  const x = Math.floor(p.x);
  const y = Math.floor(p.y);
  const z = Math.floor(p.z);
  const key = editKey(x, y, z);
  if (state.edits[key] >= 0) return;
  state.edits[key] = id;
  state.inventory[id] -= 1;
  if (id === 4) {
    state.claimed[state.planetIndex] = true;
    toast('REGISTRY BEACON FILED — CLAIM LOCKED', 3);
  } else {
    toast(`PLACED ${BLOCKS[id].name}`, 1);
  }
  rebuildEdits();
  renderHotbar();
  updateHud();
}

function updateHud() {
  $('bar-vit').style.transform = `scaleX(${state.vit / 100})`;
  $('bar-o2').style.transform = `scaleX(${state.o2 / 100})`;
  $('bar-eng').style.transform = `scaleX(${state.eng / 100})`;
  $('bar-hng').style.transform = `scaleX(${state.hng / 100})`;
  $('suspicion').textContent = state.suspicion.toFixed(1).padStart(5, '0');
  $('claim').textContent = state.claimed[state.planetIndex] ? 'CLAIM: FILED' : 'CLAIM: UNFILED';
  const tod = state.worldTime % 1;
  $('time-label').textContent =
    tod < 0.2 ? 'DAWN' : tod < 0.45 ? 'DAY' : tod < 0.55 ? 'GOLDEN' : tod < 0.7 ? 'DUSK' : 'NIGHT';
  renderHotbar();
}

function updateAtmosphere(dt) {
  state.worldTime = (state.worldTime + dt * 0.008) % 1;
  const tod = state.worldTime;
  const angle = tod * Math.PI * 2 - Math.PI * 0.5;
  const sunDir = new THREE.Vector3(Math.cos(angle), Math.sin(angle) * 0.85 + 0.15, Math.sin(angle) * 0.35);
  sunDir.normalize();
  sun.position.copy(sunDir).multiplyScalar(120);
  sun.target.position.set(0, 0, 0);
  sunMesh.position.copy(sun.position);
  const day = clamp(sunDir.y * 1.2, 0.05, 1);
  sun.intensity = 0.4 + day * 1.25;
  hemi.intensity = 0.32 + day * 0.45;
  renderer.toneMappingExposure = 0.85 + day * 0.4;
  skyMat.uniforms.sunDir.value.copy(sunDir);
  skyMat.uniforms.timeOfDay.value = day;

  const pos = controls.object.position;
  particles.position.set(pos.x, pos.y, pos.z);
  const arr = particles.geometry.attributes.position.array;
  const weather = planet().weather;
  for (let i = 0; i < PARTICLE_COUNT; i++) {
    const ix = i * 3;
    if (weather === 'snow') {
      arr[ix + 1] -= dt * (1.2 + (i % 5) * 0.2);
      arr[ix] += Math.sin(state.worldTime * 20 + i) * dt * 0.4;
    } else if (weather === 'ash' || weather === 'dust') {
      arr[ix] += dt * (0.8 + (i % 3) * 0.3);
      arr[ix + 1] += Math.sin(i + state.worldTime * 10) * dt * 0.2;
    } else if (weather === 'spores') {
      arr[ix + 1] += Math.sin(state.worldTime * 8 + i) * dt * 0.6;
      arr[ix + 2] += Math.cos(state.worldTime * 6 + i) * dt * 0.4;
    } else {
      arr[ix + 1] += Math.sin(state.worldTime * 5 + i) * dt * 0.15;
    }
    if (arr[ix + 1] < -5 || arr[ix + 1] > 45 || Math.abs(arr[ix]) > 70 || Math.abs(arr[ix + 2]) > 70) {
      arr[ix] = (Math.random() - 0.5) * 100;
      arr[ix + 1] = Math.random() * 35;
      arr[ix + 2] = (Math.random() - 0.5) * 100;
    }
  }
  particles.geometry.attributes.position.needsUpdate = true;
}

function updatePlayer(dt) {
  const speed = (keys.sprint ? 14 : 7.5) * (state.eng > 5 ? 1 : 0.55);
  direction.set(0, 0, 0);
  if (keys.forward) direction.z -= 1;
  if (keys.back) direction.z += 1;
  if (keys.left) direction.x -= 1;
  if (keys.right) direction.x += 1;
  if (direction.lengthSq() > 0) direction.normalize();

  const forward = new THREE.Vector3();
  const right = new THREE.Vector3();
  controls.getDirection(forward);
  forward.y = 0;
  forward.normalize();
  right.crossVectors(forward, new THREE.Vector3(0, 1, 0)).normalize();

  velocity.x = forward.x * -direction.z * speed + right.x * direction.x * speed;
  velocity.z = forward.z * -direction.z * speed + right.z * direction.x * speed;
  velocity.y -= 22 * dt;
  if (keys.jump && canJump) {
    velocity.y = 9.5;
    canJump = false;
    grounded = false;
  }

  const obj = controls.object;
  obj.position.x += velocity.x * dt;
  obj.position.z += velocity.z * dt;
  obj.position.y += velocity.y * dt;

  const half = WORLD * 0.48;
  obj.position.x = clamp(obj.position.x, -half, half);
  obj.position.z = clamp(obj.position.z, -half, half);

  const ground = surfaceY(obj.position.x, obj.position.z) + 1.7;
  if (obj.position.y < ground) {
    obj.position.y = ground;
    velocity.y = 0;
    canJump = true;
    grounded = true;
  }

  if (direction.lengthSq() > 0) state.eng = clamp(state.eng - dt * (keys.sprint ? 4 : 1.2), 0, 100);
  else state.eng = clamp(state.eng + dt * 3.5, 0, 100);
  state.hng = clamp(state.hng - dt * 0.35, 0, 100);
  state.o2 = clamp(state.o2 - planet().oxygenDrain * dt, 0, 100);
  if (planet().oxygenDrain > 0 && state.o2 < 5) state.vit = clamp(state.vit - dt * 6, 0, 100);
  if (planet().hazard > 0) state.vit = clamp(state.vit - planet().hazard * dt, 0, 100);
  if (state.hng < 1) state.vit = clamp(state.vit - dt * 2, 0, 100);
  if (grounded && state.o2 < 100 && planet().oxygenDrain === 0) state.o2 = clamp(state.o2 + dt * 8, 0, 100);

  state.positions[state.planetIndex] = {
    x: obj.position.x,
    y: obj.position.y,
    z: obj.position.z,
  };
}

function updateEnemies(dt) {
  const player = controls.object.position;
  let nearest = Infinity;
  for (const e of state.enemies) {
    e.phase += dt;
    const to = player.clone().sub(e.mesh.position);
    const dist = to.length();
    nearest = Math.min(nearest, dist);
    if (dist > 2.2 && dist < VIEW) {
      to.y = 0;
      to.normalize();
      e.mesh.position.addScaledVector(to, e.speed * dt);
      e.mesh.position.y = surfaceY(e.mesh.position.x, e.mesh.position.z) + 1.1;
      e.mesh.lookAt(player.x, e.mesh.position.y, player.z);
    }
    if (dist < 1.6) {
      state.vit = clamp(state.vit - dt * 12, 0, 100);
      $('interact').textContent = 'HOSTILE CONTACT — F TO STRIKE';
    }
    e.mesh.children[0].position.y = Math.sin(e.phase * 4) * 0.05;
  }
  if (nearest > 2.5) $('interact').textContent = '';
}

function tryAttack() {
  const player = controls.object.position;
  let hit = null;
  let best = 3.2;
  for (const e of state.enemies) {
    const d = e.mesh.position.distanceTo(player);
    if (d < best) {
      best = d;
      hit = e;
    }
  }
  if (!hit) return;
  hit.hp -= 18;
  state.eng = clamp(state.eng - 4, 0, 100);
  raiseSuspicion(0.8);
  if (hit.hp <= 0) {
    enemyGroup.remove(hit.mesh);
    state.enemies = state.enemies.filter((e) => e !== hit);
    toast('IMPERIAL UNIT DOWN', 1.5);
  } else {
    toast('STRIKE CONNECTED', 0.8);
  }
}

let last = performance.now();
function frame(now) {
  const dt = Math.min(0.05, (now - last) / 1000);
  last = now;

  if (state.mode === 'play') {
    updateAtmosphere(dt);
    updatePlayer(dt);
    updateEnemies(dt);
    updateHeroes(dt);
    const hero = nearestHero();
    if (hero && (!state.enemies.length || true)) {
      const label = hero.userData.label || 'FIELD UNIT';
      if (!$('interact').textContent.includes('HOSTILE')) {
        $('interact').textContent = `E · ${label}`;
      }
    }
    if (state.messageTimer > 0) {
      state.messageTimer -= dt;
      if (state.messageTimer <= 0) $('toast').classList.remove('show');
    }
    updateHud();
    if (Math.floor(now / 5000) !== Math.floor((now - dt * 1000) / 5000)) persist();
  } else if (state.mode === 'title') {
    state.worldTime = (state.worldTime + dt * 0.02) % 1;
    updateAtmosphere(dt);
    camera.position.set(Math.cos(now * 0.00015) * 60, 28, Math.sin(now * 0.00015) * 60);
    camera.lookAt(0, 8, 0);
  }

  sky.position.copy(camera.position);
  composer.render();
  requestAnimationFrame(frame);
}

function startGame(continueSave) {
  try {
    if (continueSave) loadSave();
    else {
      localStorage.removeItem(SAVE_KEY);
      state.edits = {};
      state.claimed = Array(PLANETS.length).fill(false);
      state.suspicion = 0;
      state.vit = state.o2 = state.eng = state.hng = 100;
    state.worldTime = 0.3; // morning showcase light for ceramic/gold
      state.positions = PLANETS.map(() => null);
      state.inventory = Object.fromEntries(BLOCKS.map((b) => [b.id, b.id === 0 ? 24 : b.id < 4 ? 12 : 4]));
    }
    // Build world before pointer-lock so a lock failure cannot strand the title screen
    state.mode = 'play';
    buildTerrain();
    spawnHeroes();
    spawnPlayer(!continueSave);
    setMode('play');
    toast(continueSave ? 'SAVE RELOADED // HERO CAMP ONLINE' : 'UNFILED. Hero camp on LZ — white ceramic units nearby. Press E.', 3.5);
    updateHud();
  } catch (err) {
    console.error(err);
    $('loading')?.classList.add('hidden');
    toast('BOOT FAULT · ' + (err.message || err), 6);
    setMode('title');
  }
}

window.__elysiumNew = () => startGame(false);
window.__elysiumContinue = () => startGame(true);
$('btn-new').onclick = window.__elysiumNew;
$('btn-continue').onclick = window.__elysiumContinue;
$('btn-resume').onclick = () => setMode('play');
$('btn-title').onclick = () => {
  persist();
  setMode('title');
};
$('btn-travel').onclick = () => {
  renderTravel();
  setMode('travel');
};
$('btn-travel-close').onclick = () => setMode('pause');
$('btn-map').onclick = () => {
  renderMap();
  setMode('map');
};
$('btn-map-close').onclick = () => setMode('pause');
if ($('btn-craft-close')) $('btn-craft-close').onclick = () => setMode('pause');

addEventListener('resize', () => {
  camera.aspect = innerWidth / innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(innerWidth, innerHeight);
  composer.setSize(innerWidth, innerHeight);
  bloomPass.setSize(innerWidth, innerHeight);
});

addEventListener('keydown', (e) => {
  if (e.code === 'KeyW') keys.forward = true;
  if (e.code === 'KeyS') keys.back = true;
  if (e.code === 'KeyA') keys.left = true;
  if (e.code === 'KeyD') keys.right = true;
  if (e.code === 'ShiftLeft') keys.sprint = true;
  if (e.code === 'Space') keys.jump = true;
  if (state.mode === 'play') {
    if (e.code === 'Escape') setMode('pause');
    if (e.code === 'KeyT') {
      renderTravel();
      setMode('travel');
    }
    if (e.code === 'KeyM') {
      renderMap();
      setMode('map');
    }
    if (e.code === 'KeyF') tryAttack();
    if (e.code === 'KeyE') {
      const hero = nearestHero();
      if (hero) {
        toast(`${hero.userData.label || 'UNIT'} · SYSTEMS ONLINE`, 2.2);
        if (hero.userData.kind === 'fabricator') {
          raiseSuspicion(1.5);
          state.inventory[3] = (state.inventory[3] ?? 0) + 1;
          renderHotbar();
          toast('FAB-01 OUTPUT · +1 STEEL', 2.0);
        }
      } else {
        state.vit = clamp(state.vit + 8, 0, 100);
        state.o2 = clamp(state.o2 + 12, 0, 100);
        state.hng = clamp(state.hng + 10, 0, 100);
        toast('FIELD RATION / SUIT CYCLE', 1.5);
        updateHud();
      }
    }
    if (e.code === 'KeyC') {
      renderCraft();
      setMode('craft');
    }
    const num = Number(e.key);
    if (num >= 1 && num <= 8) {
      state.selected = num - 1;
      renderHotbar();
    }
  } else if (e.code === 'Escape') {
    if (state.mode === 'pause') setMode('play');
    else if (state.mode === 'travel' || state.mode === 'map' || state.mode === 'craft') setMode('pause');
  }
});

addEventListener('keyup', (e) => {
  if (e.code === 'KeyW') keys.forward = false;
  if (e.code === 'KeyS') keys.back = false;
  if (e.code === 'KeyA') keys.left = false;
  if (e.code === 'KeyD') keys.right = false;
  if (e.code === 'ShiftLeft') keys.sprint = false;
  if (e.code === 'Space') keys.jump = false;
});

addEventListener('mousedown', (e) => {
  if (state.mode !== 'play') return;
  if (!controls.isLocked) {
    controls.lock();
    return;
  }
  if (e.button === 0) tryMine();
  if (e.button === 2) tryPlace();
});

addEventListener('contextmenu', (e) => e.preventDefault());

controls.addEventListener('lock', () => {
  if (state.mode === 'play') $('hud').classList.remove('hidden');
});

// Title-orbit preview terrain
state.planetIndex = 0;
buildTerrain();
camera.position.set(40, 24, 40);
camera.lookAt(0, 8, 0);
requestAnimationFrame(frame);
setMode('title');
