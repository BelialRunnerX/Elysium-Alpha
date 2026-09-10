import { WORLD, SEA } from './config.js';
import { heightAt, hash } from './noise.js';
import { bindThree, textureForPlanet, rockTexture, magmaTexture } from './textures.js';

export function createWorld(THREE, planet) {
  bindThree(THREE);
  const group = new THREE.Group();
  const tex = textureForPlanet(planet);
  tex.repeat.set(28, 28);
  const rockTex = rockTexture();
  rockTex.repeat.set(10, 10);

  const seg = 160;
  const geo = new THREE.PlaneGeometry(WORLD, WORLD, seg, seg);
  geo.rotateX(-Math.PI / 2);
  const pos = geo.attributes.position;
  const colors = new Float32Array(pos.count * 3);
  const gCol = new THREE.Color(...planet.ground);
  const rCol = new THREE.Color(...planet.rock);
  const aCol = new THREE.Color(...planet.accent);

  for (let i = 0; i < pos.count; i++) {
    const x = pos.getX(i);
    const z = pos.getZ(i);
    const h = heightAt(x, z, planet);
    pos.setY(i, h);
    const slope =
      Math.abs(heightAt(x + 1.2, z, planet) - heightAt(x - 1.2, z, planet)) +
      Math.abs(heightAt(x, z + 1.2, planet) - heightAt(x, z - 1.2, planet));
    const c = gCol.clone().lerp(rCol, THREE.MathUtils.clamp(slope * 0.16, 0, 1));
    if (planet.key === 'temperate' && h > SEA + 1.2 && slope < 1.1) c.lerp(aCol, 0.2);
    if (planet.key === 'scorched' && h < SEA) c.setRGB(0.9, 0.28, 0.08);
    if (planet.key === 'toxic') c.lerp(aCol, 0.15);
    colors[i * 3] = c.r;
    colors[i * 3 + 1] = c.g;
    colors[i * 3 + 2] = c.b;
  }
  geo.setAttribute('color', new THREE.BufferAttribute(colors, 3));
  geo.computeVertexNormals();

  const terrain = new THREE.Mesh(
    geo,
    new THREE.MeshStandardMaterial({
      map: tex,
      vertexColors: true,
      roughness: 0.9,
      metalness: 0.04,
    }),
  );
  terrain.receiveShadow = true;
  terrain.castShadow = true;
  terrain.name = 'terrain';
  group.add(terrain);

  if (planet.key === 'oceanic' || planet.key === 'temperate' || planet.key === 'frozen') {
    const liquid = new THREE.Mesh(
      new THREE.PlaneGeometry(WORLD * 1.25, WORLD * 1.25),
      new THREE.MeshStandardMaterial({
        color: planet.key === 'frozen' ? 0xa9c8d8 : 0x1c5f78,
        transparent: true,
        opacity: 0.68,
        roughness: 0.12,
        metalness: 0.4,
      }),
    );
    liquid.rotation.x = -Math.PI / 2;
    liquid.position.y = SEA - (planet.key === 'oceanic' ? 0.15 : 2.4);
    group.add(liquid);
  } else if (planet.key === 'scorched') {
    const liquid = new THREE.Mesh(
      new THREE.PlaneGeometry(WORLD * 0.35, WORLD * 0.35),
      new THREE.MeshStandardMaterial({
        map: magmaTexture(),
        emissive: 0xff4400,
        emissiveIntensity: 0.85,
        roughness: 0.6,
      }),
    );
    liquid.rotation.x = -Math.PI / 2;
    liquid.position.set(18, SEA - 3.5, -12);
    group.add(liquid);
  }

  const rockGeo = new THREE.DodecahedronGeometry(0.55, 0);
  const rockMat = new THREE.MeshStandardMaterial({ map: rockTex, roughness: 0.95, flatShading: true });
  const rockMesh = new THREE.InstancedMesh(rockGeo, rockMat, 180);
  rockMesh.castShadow = true;
  const dummy = new THREE.Object3D();
  let ri = 0;
  for (let i = 0; i < 180; i++) {
    const x = (hash(i, 3, planet.seed) - 0.5) * WORLD * 0.9;
    const z = (hash(i, 9, planet.seed) - 0.5) * WORLD * 0.9;
    const y = heightAt(x, z, planet);
    if (planet.key === 'oceanic' && y < SEA) continue;
    dummy.position.set(x, y + 0.2, z);
    dummy.scale.setScalar(0.5 + hash(i, 1, planet.seed) * 1.8);
    dummy.rotation.set(hash(i, 2, planet.seed) * 2, hash(i, 4, planet.seed) * 6, 0);
    dummy.updateMatrix();
    rockMesh.setMatrixAt(ri++, dummy.matrix);
  }
  rockMesh.count = ri;
  group.add(rockMesh);

  if (planet.key === 'temperate' || planet.key === 'toxic') {
    const stem = new THREE.CylinderGeometry(0.05, 0.08, 1.2, 5);
    const leaf = new THREE.ConeGeometry(0.45, 1.1, 6);
    const stemMat = new THREE.MeshStandardMaterial({ color: 0x4a3420 });
    const leafMat = new THREE.MeshStandardMaterial({
      color: planet.key === 'toxic' ? 0x9cff4a : 0x2f8f4a,
      roughness: 0.8,
    });
    for (let i = 0; i < 70; i++) {
      const x = (hash(i, 11, planet.seed) - 0.5) * WORLD * 0.85;
      const z = (hash(i, 17, planet.seed) - 0.5) * WORLD * 0.85;
      const y = heightAt(x, z, planet);
      if (y < SEA + 0.5) continue;
      const t = new THREE.Group();
      const s = new THREE.Mesh(stem, stemMat);
      s.position.y = 0.6;
      const l = new THREE.Mesh(leaf, leafMat);
      l.position.y = 1.35;
      t.add(s, l);
      t.position.set(x, y, z);
      t.scale.setScalar(0.7 + hash(i, 5, planet.seed));
      group.add(t);
    }
  }

  return { group, terrain };
}

export function buildEnemy(THREE, archetype = 'drone') {
  const root = new THREE.Group();
  const bodyMat = new THREE.MeshStandardMaterial({ color: 0x1a221f, metalness: 0.45, roughness: 0.4 });
  const accent = new THREE.MeshStandardMaterial({
    color: archetype === 'praetor' ? 0xff6b4a : 0x4affc0,
    emissive: archetype === 'praetor' ? 0xff3a20 : 0x4affc0,
    emissiveIntensity: 1.5,
  });
  root.add(new THREE.Mesh(new THREE.CapsuleGeometry(0.32, 0.55, 4, 8), bodyMat));
  const eye = new THREE.Mesh(new THREE.SphereGeometry(0.12, 12, 12), accent);
  eye.position.set(0, 0.45, 0.25);
  root.add(eye);
  if (archetype !== 'drone') {
    const wings = new THREE.Mesh(new THREE.BoxGeometry(1.2, 0.08, 0.35), bodyMat);
    wings.position.y = 0.35;
    root.add(wings);
  }
  root.traverse((o) => {
    if (o.isMesh) o.castShadow = true;
  });
  return root;
}
