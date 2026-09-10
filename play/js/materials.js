/**
 * Elysium Art Bible — PBR material kit
 * Palette locked to concept sheets:
 *   Ceramic Composite (Suit White)
 *   Graphite Metal
 *   Solar-Gold Accent
 *   Emissive Cyan
 */
let THREE;

export function bindMaterials(T) {
  THREE = T;
}

function ensure() {
  if (!THREE) throw new Error('bindMaterials(THREE) required before use');
}

function panelNormalMap(size = 256, line = 18) {
  ensure();
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#8080ff';
  ctx.fillRect(0, 0, size, size);
  ctx.strokeStyle = 'rgba(60,60,200,0.55)';
  ctx.lineWidth = 2;
  const step = Math.floor(size / line);
  for (let i = 0; i <= size; i += step) {
    ctx.beginPath();
    ctx.moveTo(i, 0);
    ctx.lineTo(i, size);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(0, i);
    ctx.lineTo(size, i);
    ctx.stroke();
  }
  // rivets
  ctx.fillStyle = 'rgba(200,200,255,0.35)';
  for (let y = step / 2; y < size; y += step) {
    for (let x = step / 2; x < size; x += step) {
      ctx.beginPath();
      ctx.arc(x, y, 1.6, 0, Math.PI * 2);
      ctx.fill();
    }
  }
  const tex = new THREE.CanvasTexture(c);
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.colorSpace = THREE.NoColorSpace;
  return tex;
}

function brushedMetalMap(size = 256) {
  ensure();
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#2a2d32';
  ctx.fillRect(0, 0, size, size);
  for (let y = 0; y < size; y++) {
    const v = 30 + Math.random() * 40;
    ctx.fillStyle = `rgb(${v},${v + 2},${v + 4})`;
    ctx.fillRect(0, y, size, 1);
  }
  const tex = new THREE.CanvasTexture(c);
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

function ceramicMap(size = 256) {
  ensure();
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const ctx = c.getContext('2d');
  ctx.fillStyle = '#f2f4f6';
  ctx.fillRect(0, 0, size, size);
  for (let i = 0; i < 400; i++) {
    ctx.fillStyle = `rgba(220,225,230,${0.15 + Math.random() * 0.25})`;
    ctx.fillRect(Math.random() * size, Math.random() * size, 2, 2);
  }
  const tex = new THREE.CanvasTexture(c);
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

let cache;

export function elysiumMaterials() {
  ensure();
  if (cache) return cache;
  const panelN = panelNormalMap();
  const ceramic = ceramicMap();
  const graphiteMap = brushedMetalMap();

  cache = {
    ceramic: new THREE.MeshStandardMaterial({
      name: 'CeramicComposite',
      color: 0xffffff,
      map: ceramic,
      normalMap: panelN,
      normalScale: new THREE.Vector2(0.35, 0.35),
      roughness: 0.38,
      metalness: 0.06,
      envMapIntensity: 1.1,
    }),
    graphite: new THREE.MeshStandardMaterial({
      name: 'GraphiteMetal',
      color: 0x2c3036,
      map: graphiteMap,
      roughness: 0.55,
      metalness: 0.85,
    }),
    gold: new THREE.MeshStandardMaterial({
      name: 'SolarGold',
      color: 0xe0b93a,
      roughness: 0.22,
      metalness: 1.0,
      emissive: 0x3a2808,
      emissiveIntensity: 0.15,
    }),
    emissive: new THREE.MeshStandardMaterial({
      name: 'EmissiveCyan',
      color: 0x1a3a40,
      emissive: 0x3de0ff,
      emissiveIntensity: 2.4,
      roughness: 0.35,
      metalness: 0.2,
    }),
    rubber: new THREE.MeshStandardMaterial({
      name: 'AntiSlipRubber',
      color: 0x1a1a1a,
      roughness: 0.95,
      metalness: 0.0,
    }),
    glass: new THREE.MeshPhysicalMaterial({
      name: 'LensGlass',
      color: 0x88e8ff,
      transmission: 0.55,
      transparent: true,
      opacity: 0.85,
      roughness: 0.05,
      metalness: 0.0,
      thickness: 0.4,
      emissive: 0x1ad0ff,
      emissiveIntensity: 0.6,
    }),
  };
  return cache;
}

/** Shared helper: box with ceramic shell + graphite trim accents. */
export function armoredBox(w, h, d, mats, opts = {}) {
  ensure();
  const g = new THREE.Group();
  const body = new THREE.Mesh(new THREE.BoxGeometry(w, h, d), mats.ceramic);
  body.castShadow = body.receiveShadow = true;
  g.add(body);

  // panel seam rings
  const trim = new THREE.Mesh(
    new THREE.BoxGeometry(w * 1.01, h * 0.06, d * 1.01),
    mats.graphite,
  );
  trim.position.y = h * 0.28;
  trim.castShadow = true;
  g.add(trim);

  if (opts.goldRails) {
    for (const side of [-1, 1]) {
      const rail = new THREE.Mesh(
        new THREE.CapsuleGeometry(0.03, w * 0.55, 4, 8),
        mats.gold,
      );
      rail.rotation.z = Math.PI / 2;
      rail.position.set(0, h * 0.55, side * d * 0.42);
      g.add(rail);
    }
  }
  return g;
}

export function boltRing(radius, mats, count = 8) {
  ensure();
  const g = new THREE.Group();
  for (let i = 0; i < count; i++) {
    const a = (i / count) * Math.PI * 2;
    const bolt = new THREE.Mesh(new THREE.CylinderGeometry(0.02, 0.02, 0.04, 6), mats.gold);
    bolt.rotation.x = Math.PI / 2;
    bolt.position.set(Math.cos(a) * radius, 0, Math.sin(a) * radius);
    g.add(bolt);
  }
  return g;
}

/** Stencil-style hull marking (ELYSIUM / unit codes) matching concept sheets. */
export function hullDecal(text, w = 0.42, h = 0.12) {
  ensure();
  const c = document.createElement('canvas');
  c.width = 512;
  c.height = 128;
  const ctx = c.getContext('2d');
  ctx.clearRect(0, 0, c.width, c.height);
  ctx.fillStyle = 'rgba(20, 28, 34, 0.0)';
  ctx.fillRect(0, 0, c.width, c.height);
  ctx.strokeStyle = 'rgba(45, 52, 58, 0.85)';
  ctx.lineWidth = 4;
  ctx.strokeRect(12, 18, c.width - 24, c.height - 36);
  ctx.fillStyle = '#2c3036';
  ctx.font = '700 52px "IBM Plex Sans", sans-serif';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(text, c.width / 2, c.height / 2);
  const tex = new THREE.CanvasTexture(c);
  tex.colorSpace = THREE.SRGBColorSpace;
  const mat = new THREE.MeshBasicMaterial({
    map: tex,
    transparent: true,
    depthWrite: false,
  });
  mat.name = 'HullDecal';
  const mesh = new THREE.Mesh(new THREE.PlaneGeometry(w, h), mat);
  return mesh;
}

export function statusStrip(len, mats) {
  ensure();
  const strip = new THREE.Mesh(new THREE.BoxGeometry(len, 0.03, 0.04), mats.emissive.clone());
  strip.userData.pulse = true;
  return strip;
}
