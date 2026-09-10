/**
 * Elysium Art Bible — PBR material kit (concept-sheet locked)
 * Ceramic Composite · Graphite Metal · Solar-Gold · Emissive Cyan
 */
let THREE;
let cache;

export function bindMaterials(T) {
  THREE = T;
  cache = null;
}

function ensure() {
  if (!THREE) throw new Error('bindMaterials(THREE) required before use');
}

function makeCanvas(size, draw) {
  ensure();
  const c = document.createElement('canvas');
  c.width = c.height = size;
  draw(c.getContext('2d'), size);
  const tex = new THREE.CanvasTexture(c);
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.anisotropy = 8;
  return tex;
}

function panelNormalMap(size = 512, line = 24) {
  const tex = makeCanvas(size, (ctx, s) => {
    ctx.fillStyle = '#8080ff';
    ctx.fillRect(0, 0, s, s);
    const step = Math.floor(s / line);
    ctx.strokeStyle = 'rgba(40,40,180,0.65)';
    ctx.lineWidth = 3;
    for (let i = 0; i <= s; i += step) {
      ctx.beginPath();
      ctx.moveTo(i, 0);
      ctx.lineTo(i, s);
      ctx.stroke();
      ctx.beginPath();
      ctx.moveTo(0, i);
      ctx.lineTo(s, i);
      ctx.stroke();
    }
    // micro rivets
    ctx.fillStyle = 'rgba(220,220,255,0.45)';
    for (let y = step / 2; y < s; y += step) {
      for (let x = step / 2; x < s; x += step) {
        ctx.beginPath();
        ctx.arc(x, y, 2.2, 0, Math.PI * 2);
        ctx.fill();
      }
    }
    // secondary seams
    ctx.strokeStyle = 'rgba(70,70,200,0.35)';
    ctx.lineWidth = 1.5;
    for (let i = step / 2; i < s; i += step) {
      ctx.beginPath();
      ctx.moveTo(i, 0);
      ctx.lineTo(i, s);
      ctx.stroke();
    }
  });
  tex.colorSpace = THREE.NoColorSpace;
  return tex;
}

function ceramicAlbedo(size = 512) {
  const tex = makeCanvas(size, (ctx, s) => {
    const g = ctx.createLinearGradient(0, 0, s, s);
    g.addColorStop(0, '#f7f9fb');
    g.addColorStop(0.5, '#eef1f4');
    g.addColorStop(1, '#e6eaee');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    // soft panel variation
    for (let i = 0; i < 90; i++) {
      const x = Math.random() * s;
      const y = Math.random() * s;
      const w = 20 + Math.random() * 80;
      ctx.fillStyle = `rgba(255,255,255,${0.04 + Math.random() * 0.08})`;
      ctx.fillRect(x, y, w, w * 0.6);
    }
    // micro speckles / wear
    for (let i = 0; i < 1200; i++) {
      ctx.fillStyle = `rgba(180,190,200,${0.05 + Math.random() * 0.12})`;
      ctx.fillRect(Math.random() * s, Math.random() * s, 1 + Math.random() * 2, 1);
    }
  });
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

function graphiteAlbedo(size = 512) {
  const tex = makeCanvas(size, (ctx, s) => {
    ctx.fillStyle = '#2a2e34';
    ctx.fillRect(0, 0, s, s);
    for (let y = 0; y < s; y++) {
      const v = 28 + Math.random() * 48;
      ctx.fillStyle = `rgb(${v},${v + 2},${v + 5})`;
      ctx.fillRect(0, y, s, 1);
    }
    // machined grooves
    ctx.strokeStyle = 'rgba(0,0,0,0.25)';
    ctx.lineWidth = 1;
    for (let i = 0; i < s; i += 8) {
      ctx.beginPath();
      ctx.moveTo(0, i);
      ctx.lineTo(s, i);
      ctx.stroke();
    }
  });
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

function goldAlbedo(size = 256) {
  const tex = makeCanvas(size, (ctx, s) => {
    const g = ctx.createLinearGradient(0, 0, s, s);
    g.addColorStop(0, '#f0d078');
    g.addColorStop(0.45, '#d4a834');
    g.addColorStop(1, '#a87a1c');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    for (let y = 0; y < s; y++) {
      const a = 0.04 + Math.random() * 0.1;
      ctx.fillStyle = `rgba(255,240,180,${a})`;
      ctx.fillRect(0, y, s, 1);
    }
  });
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

function rubberAlbedo(size = 256) {
  const tex = makeCanvas(size, (ctx, s) => {
    ctx.fillStyle = '#141414';
    ctx.fillRect(0, 0, s, s);
    ctx.strokeStyle = 'rgba(60,60,60,0.7)';
    ctx.lineWidth = 2;
    for (let i = 0; i < s; i += 10) {
      for (let j = 0; j < s; j += 10) {
        ctx.strokeRect(i, j, 8, 8);
      }
    }
  });
  tex.colorSpace = THREE.SRGBColorSpace;
  return tex;
}

export function elysiumMaterials() {
  ensure();
  if (cache) return cache;
  const panelN = panelNormalMap();
  const ceramic = ceramicAlbedo();
  const graphiteMap = graphiteAlbedo();
  const goldMap = goldAlbedo();
  const rubberMap = rubberAlbedo();

  cache = {
    ceramic: new THREE.MeshStandardMaterial({
      name: 'CeramicComposite',
      color: 0xffffff,
      map: ceramic,
      normalMap: panelN,
      normalScale: new THREE.Vector2(0.55, 0.55),
      roughness: 0.36,
      metalness: 0.05,
      envMapIntensity: 1.15,
    }),
    graphite: new THREE.MeshStandardMaterial({
      name: 'GraphiteMetal',
      color: 0x3a4048,
      map: graphiteMap,
      roughness: 0.52,
      metalness: 0.9,
      envMapIntensity: 1.0,
    }),
    gold: new THREE.MeshStandardMaterial({
      name: 'SolarGold',
      color: 0xffd24a,
      map: goldMap,
      roughness: 0.2,
      metalness: 1.0,
      emissive: 0x4a3208,
      emissiveIntensity: 0.18,
      envMapIntensity: 1.4,
    }),
    emissive: new THREE.MeshStandardMaterial({
      name: 'EmissiveCyan',
      color: 0x163840,
      emissive: 0x3de0ff,
      emissiveIntensity: 3.2,
      roughness: 0.3,
      metalness: 0.15,
      toneMapped: false,
    }),
    rubber: new THREE.MeshStandardMaterial({
      name: 'AntiSlipRubber',
      color: 0x1a1a1a,
      map: rubberMap,
      roughness: 0.96,
      metalness: 0.0,
    }),
    glass: new THREE.MeshPhysicalMaterial({
      name: 'LensGlass',
      color: 0xa8f0ff,
      transmission: 0.65,
      transparent: true,
      opacity: 0.9,
      roughness: 0.04,
      metalness: 0.0,
      thickness: 0.5,
      emissive: 0x1ad0ff,
      emissiveIntensity: 0.85,
      toneMapped: false,
    }),
  };
  return cache;
}

/** Multi-plate armored shell — denser panel language from the sheets. */
export function armoredBox(w, h, d, mats, opts = {}) {
  ensure();
  const g = new THREE.Group();
  const body = new THREE.Mesh(new THREE.BoxGeometry(w, h, d), mats.ceramic);
  body.castShadow = body.receiveShadow = true;
  g.add(body);

  // graphite under-frame lip
  const lip = new THREE.Mesh(
    new THREE.BoxGeometry(w * 1.02, h * 0.08, d * 1.02),
    mats.graphite,
  );
  lip.position.y = -h * 0.42;
  lip.castShadow = true;
  g.add(lip);

  // horizontal panel belts
  for (const t of [-0.22, 0.08, 0.32]) {
    const belt = new THREE.Mesh(
      new THREE.BoxGeometry(w * 1.015, h * 0.035, d * 1.015),
      mats.graphite,
    );
    belt.position.y = h * t;
    g.add(belt);
  }

  // corner bolts
  for (const sx of [-1, 1]) {
    for (const sz of [-1, 1]) {
      for (const sy of [-0.35, 0.35]) {
        const bolt = new THREE.Mesh(
          new THREE.CylinderGeometry(0.018, 0.018, 0.03, 6),
          mats.gold,
        );
        bolt.rotation.x = Math.PI / 2;
        bolt.position.set(sx * w * 0.46, h * sy, sz * d * 0.46);
        g.add(bolt);
      }
    }
  }

  if (opts.goldRails) {
    for (const side of [-1, 1]) {
      const rail = new THREE.Mesh(
        new THREE.CapsuleGeometry(0.028, w * 0.52, 4, 8),
        mats.gold,
      );
      rail.rotation.z = Math.PI / 2;
      rail.position.set(0, h * 0.58, side * d * 0.42);
      rail.castShadow = true;
      g.add(rail);
      // rail mounts
      for (const mx of [-0.35, 0.35]) {
        const mount = new THREE.Mesh(
          new THREE.BoxGeometry(0.06, 0.05, 0.06),
          mats.graphite,
        );
        mount.position.set(mx * w, h * 0.52, side * d * 0.42);
        g.add(mount);
      }
    }
  }

  if (opts.sideVents) {
    for (const side of [-1, 1]) {
      for (let i = 0; i < 4; i++) {
        const vent = new THREE.Mesh(
          new THREE.BoxGeometry(0.02, h * 0.08, d * 0.12),
          mats.graphite,
        );
        vent.position.set(side * w * 0.51, h * (-0.1 + i * 0.12), 0);
        g.add(vent);
      }
    }
  }

  return g;
}

export function boltRing(radius, mats, count = 8) {
  ensure();
  const g = new THREE.Group();
  for (let i = 0; i < count; i++) {
    const a = (i / count) * Math.PI * 2;
    const bolt = new THREE.Mesh(new THREE.CylinderGeometry(0.018, 0.018, 0.035, 6), mats.gold);
    bolt.rotation.x = Math.PI / 2;
    bolt.position.set(Math.cos(a) * radius, 0, Math.sin(a) * radius);
    g.add(bolt);
  }
  return g;
}

export function goldJoint(radius, mats) {
  ensure();
  const g = new THREE.Group();
  const ball = new THREE.Mesh(new THREE.SphereGeometry(radius, 14, 14), mats.gold);
  ball.castShadow = true;
  const ring = new THREE.Mesh(
    new THREE.TorusGeometry(radius * 1.05, radius * 0.18, 8, 20),
    mats.graphite,
  );
  g.add(ball, ring);
  return g;
}

/** Stencil hull marking matching concept sheets. */
export function hullDecal(text, w = 0.42, h = 0.12) {
  ensure();
  const c = document.createElement('canvas');
  c.width = 1024;
  c.height = 256;
  const ctx = c.getContext('2d');
  ctx.clearRect(0, 0, c.width, c.height);
  ctx.strokeStyle = 'rgba(35, 42, 48, 0.9)';
  ctx.lineWidth = 6;
  ctx.strokeRect(18, 28, c.width - 36, c.height - 56);
  ctx.fillStyle = '#1e242a';
  ctx.font = '700 92px "IBM Plex Sans", "Orbitron", sans-serif';
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  ctx.fillText(text, c.width / 2, c.height / 2);
  // micro code line
  ctx.font = '500 28px "IBM Plex Sans", sans-serif';
  ctx.fillStyle = 'rgba(30,36,42,0.55)';
  ctx.fillText('ELYSIUM FIELD UNIT', c.width / 2, c.height * 0.82);
  const tex = new THREE.CanvasTexture(c);
  tex.colorSpace = THREE.SRGBColorSpace;
  const mat = new THREE.MeshBasicMaterial({
    map: tex,
    transparent: true,
    depthWrite: false,
  });
  mat.name = 'HullDecal';
  return new THREE.Mesh(new THREE.PlaneGeometry(w, h), mat);
}

export function statusStrip(len, mats) {
  ensure();
  const strip = new THREE.Mesh(new THREE.BoxGeometry(len, 0.028, 0.04), mats.emissive.clone());
  strip.userData.pulse = true;
  return strip;
}

/** Ceramic plate overlay for armored limbs. */
export function armorPlate(w, h, d, mats) {
  ensure();
  const plate = new THREE.Mesh(new THREE.BoxGeometry(w, h, d), mats.ceramic);
  plate.castShadow = plate.receiveShadow = true;
  return plate;
}
