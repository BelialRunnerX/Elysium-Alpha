/** Procedural canvas textures — no external art packs required. */
let THREE;

export function bindThree(T) {
  THREE = T;
}

export function makeTexture(draw, size = 256) {
  if (!THREE) throw new Error('bindThree(THREE) first');
  const c = document.createElement('canvas');
  c.width = c.height = size;
  const ctx = c.getContext('2d');
  draw(ctx, size);
  const tex = new THREE.CanvasTexture(c);
  tex.wrapS = tex.wrapT = THREE.RepeatWrapping;
  tex.colorSpace = THREE.SRGBColorSpace;
  tex.anisotropy = 8;
  return tex;
}

export function grassTexture() {
  return makeTexture((ctx, s) => {
    const g = ctx.createLinearGradient(0, 0, s, s);
    g.addColorStop(0, '#3a6b3a');
    g.addColorStop(0.5, '#2f5a32');
    g.addColorStop(1, '#4a7a40');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    for (let i = 0; i < 1400; i++) {
      ctx.fillStyle = `rgba(${40 + Math.random() * 80},${90 + Math.random() * 90},${40 + Math.random() * 50},${0.15 + Math.random() * 0.35})`;
      ctx.fillRect(Math.random() * s, Math.random() * s, 1 + Math.random() * 2, 2 + Math.random() * 4);
    }
  });
}

export function rockTexture() {
  return makeTexture((ctx, s) => {
    ctx.fillStyle = '#6a6864';
    ctx.fillRect(0, 0, s, s);
    for (let i = 0; i < 900; i++) {
      const v = 70 + Math.random() * 70;
      ctx.fillStyle = `rgba(${v},${v - 4},${v - 8},${0.2 + Math.random() * 0.4})`;
      ctx.beginPath();
      ctx.arc(Math.random() * s, Math.random() * s, 1 + Math.random() * 6, 0, Math.PI * 2);
      ctx.fill();
    }
  });
}

export function sandTexture() {
  return makeTexture((ctx, s) => {
    ctx.fillStyle = '#c2a878';
    ctx.fillRect(0, 0, s, s);
    for (let i = 0; i < 2000; i++) {
      ctx.fillStyle = `rgba(${160 + Math.random() * 60},${130 + Math.random() * 40},${80 + Math.random() * 40},0.35)`;
      ctx.fillRect(Math.random() * s, Math.random() * s, 1, 1);
    }
  });
}

export function iceTexture() {
  return makeTexture((ctx, s) => {
    const g = ctx.createLinearGradient(0, 0, s, s);
    g.addColorStop(0, '#d8e8f2');
    g.addColorStop(1, '#a8c4d8');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    for (let i = 0; i < 80; i++) {
      ctx.strokeStyle = `rgba(255,255,255,${0.1 + Math.random() * 0.25})`;
      ctx.beginPath();
      ctx.moveTo(Math.random() * s, Math.random() * s);
      ctx.lineTo(Math.random() * s, Math.random() * s);
      ctx.stroke();
    }
  });
}

export function metalTexture() {
  return makeTexture((ctx, s) => {
    ctx.fillStyle = '#8a969c';
    ctx.fillRect(0, 0, s, s);
    for (let y = 0; y < s; y += 3) {
      ctx.fillStyle = `rgba(255,255,255,${0.02 + Math.random() * 0.06})`;
      ctx.fillRect(0, y, s, 1);
    }
  }, 128);
}

export function magmaTexture() {
  return makeTexture((ctx, s) => {
    ctx.fillStyle = '#1a0a08';
    ctx.fillRect(0, 0, s, s);
    for (let i = 0; i < 200; i++) {
      const r = 2 + Math.random() * 10;
      const g = ctx.createRadialGradient(0, 0, 0, 0, 0, r);
      g.addColorStop(0, 'rgba(255,200,80,0.9)');
      g.addColorStop(0.5, 'rgba(255,80,20,0.5)');
      g.addColorStop(1, 'rgba(40,0,0,0)');
      ctx.save();
      ctx.translate(Math.random() * s, Math.random() * s);
      ctx.fillStyle = g;
      ctx.beginPath();
      ctx.arc(0, 0, r, 0, Math.PI * 2);
      ctx.fill();
      ctx.restore();
    }
  });
}

export function textureForPlanet(planet) {
  if (planet.key === 'frozen') return iceTexture();
  if (planet.key === 'barren' || planet.key === 'scorched') return sandTexture();
  if (planet.key === 'oceanic') return sandTexture();
  return grassTexture();
}
