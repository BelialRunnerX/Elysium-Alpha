/**
 * Hero hard-surface assets matching Elysium concept sheets.
 * Palette: ceramic white / graphite / solar-gold / cyan emissive.
 */
import { elysiumMaterials, armoredBox, boltRing, hullDecal, statusStrip } from '../materials.js';

function leg(THREE, mats, sideX, sideZ) {
  const g = new THREE.Group();
  const upper = new THREE.Mesh(new THREE.CapsuleGeometry(0.06, 0.22, 4, 8), mats.graphite);
  upper.position.set(0, -0.12, 0);
  upper.rotation.z = sideX * 0.35;
  upper.rotation.x = sideZ * 0.2;
  const joint = new THREE.Mesh(new THREE.SphereGeometry(0.07, 12, 12), mats.gold);
  joint.position.set(sideX * 0.08, -0.26, sideZ * 0.05);
  const lower = new THREE.Mesh(new THREE.CapsuleGeometry(0.05, 0.2, 4, 8), mats.graphite);
  lower.position.set(sideX * 0.12, -0.4, sideZ * 0.08);
  lower.rotation.z = -sideX * 0.25;
  const foot = new THREE.Mesh(new THREE.SphereGeometry(0.09, 12, 12), mats.rubber);
  foot.scale.set(1.1, 0.55, 1.3);
  foot.position.set(sideX * 0.14, -0.55, sideZ * 0.1);
  const ring = new THREE.Mesh(new THREE.TorusGeometry(0.05, 0.012, 8, 16), mats.emissive);
  ring.position.copy(joint.position);
  ring.rotation.x = Math.PI / 2;
  g.add(upper, joint, lower, foot, ring);
  g.position.set(sideX * 0.28, 0.55, sideZ * 0.28);
  g.traverse((o) => {
    if (o.isMesh) {
      o.castShadow = true;
      o.receiveShadow = true;
    }
  });
  return g;
}

function manipulator(THREE, mats) {
  const arm = new THREE.Group();
  const shoulder = new THREE.Mesh(new THREE.CylinderGeometry(0.06, 0.06, 0.1, 10), mats.gold);
  shoulder.rotation.z = Math.PI / 2;
  const bone1 = new THREE.Mesh(new THREE.CapsuleGeometry(0.035, 0.22, 4, 8), mats.graphite);
  bone1.position.set(0.16, -0.02, 0.05);
  bone1.rotation.z = -0.5;
  bone1.rotation.y = 0.4;
  const elbow = new THREE.Mesh(new THREE.SphereGeometry(0.045, 10, 10), mats.gold);
  elbow.position.set(0.28, -0.12, 0.12);
  const bone2 = new THREE.Mesh(new THREE.CapsuleGeometry(0.03, 0.18, 4, 8), mats.graphite);
  bone2.position.set(0.36, -0.2, 0.18);
  bone2.rotation.z = -0.8;
  const wrist = new THREE.Mesh(new THREE.CylinderGeometry(0.04, 0.04, 0.06, 8), mats.ceramic);
  wrist.position.set(0.42, -0.28, 0.24);
  const claw = new THREE.Group();
  for (let i = 0; i < 3; i++) {
    const a = ((i / 3) * Math.PI * 2) - Math.PI / 2;
    const finger = new THREE.Mesh(new THREE.BoxGeometry(0.02, 0.1, 0.025), mats.graphite);
    finger.position.set(Math.cos(a) * 0.04, -0.06, Math.sin(a) * 0.04);
    finger.rotation.x = 0.5;
    const tip = new THREE.Mesh(new THREE.SphereGeometry(0.015, 8, 8), mats.emissive);
    tip.position.set(0, -0.05, 0);
    finger.add(tip);
    claw.add(finger);
  }
  claw.position.copy(wrist.position);
  claw.position.y -= 0.04;
  arm.add(shoulder, bone1, elbow, bone2, wrist, claw);
  arm.position.set(0.38, 0.35, 0.35);
  arm.traverse((o) => {
    if (o.isMesh) o.castShadow = true;
  });
  return arm;
}

/** TDR-01 Terra-Drone — quadruped utility / support. */
export function createTerraDrone(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'TerraDrone_TDR01';

  const chassis = armoredBox(0.7, 0.32, 0.55, mats, { goldRails: true });
  chassis.position.y = 0.62;
  root.add(chassis);

  // front sensor eye
  const eyeHousing = new THREE.Mesh(new THREE.CylinderGeometry(0.12, 0.14, 0.08, 20), mats.graphite);
  eyeHousing.rotation.x = Math.PI / 2;
  eyeHousing.position.set(0, 0.62, 0.3);
  const eye = new THREE.Mesh(new THREE.SphereGeometry(0.09, 20, 20), mats.glass);
  eye.position.set(0, 0.62, 0.34);
  const iris = new THREE.Mesh(new THREE.CircleGeometry(0.04, 20), mats.emissive);
  iris.position.set(0, 0.62, 0.4);
  root.add(eyeHousing, eye, iris, boltRing(0.11, mats, 6));
  root.children[root.children.length - 1].position.set(0, 0.62, 0.3);
  root.children[root.children.length - 1].rotation.x = Math.PI / 2;

  // status strips
  for (const x of [-0.28, 0.28]) {
    const strip = statusStrip(0.35, mats);
    strip.rotation.y = Math.PI / 2;
    strip.scale.set(1, 2.2, 1);
    strip.position.set(x, 0.7, 0);
    root.add(strip);
  }

  // hull markings — concept sheet decals
  const mark = hullDecal('ELYSIUM  TDR-01', 0.55, 0.1);
  mark.position.set(0, 0.72, 0.285);
  root.add(mark);
  const markSide = hullDecal('TOR-01', 0.28, 0.08);
  markSide.position.set(0.36, 0.62, 0);
  markSide.rotation.y = Math.PI / 2;
  root.add(markSide);

  // panel greebles / service ports
  for (const z of [-0.12, 0.12]) {
    const port = new THREE.Mesh(new THREE.CylinderGeometry(0.03, 0.03, 0.02, 10), mats.graphite);
    port.rotation.x = Math.PI / 2;
    port.position.set(-0.36, 0.62, z);
    root.add(port);
  }
  const handle = new THREE.Mesh(new THREE.TorusGeometry(0.08, 0.015, 8, 16, Math.PI), mats.gold);
  handle.position.set(0, 0.82, -0.05);
  handle.rotation.x = Math.PI / 2;
  root.add(handle);

  // antenna
  const ant = new THREE.Mesh(new THREE.CylinderGeometry(0.01, 0.01, 0.28, 6), mats.graphite);
  ant.position.set(-0.2, 0.9, -0.1);
  const antTip = new THREE.Mesh(new THREE.SphereGeometry(0.025, 8, 8), mats.emissive);
  antTip.position.set(-0.2, 1.05, -0.1);
  antTip.userData.pulse = true;
  root.add(ant, antTip);

  root.add(leg(THREE, mats, -1, 1));
  root.add(leg(THREE, mats, 1, 1));
  root.add(leg(THREE, mats, -1, -1));
  root.add(leg(THREE, mats, 1, -1));
  root.add(manipulator(THREE, mats));

  // rear vent
  const vent = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.12, 0.08), mats.graphite);
  vent.position.set(0, 0.62, -0.3);
  const ventGlow = new THREE.Mesh(new THREE.BoxGeometry(0.28, 0.03, 0.02), mats.emissive);
  ventGlow.position.set(0, 0.62, -0.34);
  root.add(vent, ventGlow);

  // point light for emissive presence
  const light = new THREE.PointLight(0x3de0ff, 0.9, 6);
  light.position.set(0, 0.7, 0.4);
  root.add(light);

  // build/repair holo grid (concept function mode)
  const gridMat = mats.emissive.clone();
  gridMat.transparent = true;
  gridMat.opacity = 0.35;
  gridMat.depthWrite = false;
  const grid = new THREE.GridHelper(1.2, 8, 0x3de0ff, 0x1a6a80);
  grid.position.set(0.55, 0.02, 0.55);
  root.add(grid);
  const holo = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.35, 0.35), gridMat);
  holo.position.set(0.55, 0.25, 0.55);
  holo.userData.pulse = true;
  root.add(holo);

  root.userData.kind = 'terra_drone';
  root.userData.label = 'TDR-01 TERRA-DRONE';
  return root;
}

/** R-01 Rover — compact modular ground vehicle. */
export function createRover(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'Rover_R01';

  const body = armoredBox(1.35, 0.45, 0.85, mats, { goldRails: true });
  body.position.y = 0.55;
  root.add(body);

  // cabin glass
  const cabin = new THREE.Mesh(new THREE.BoxGeometry(0.55, 0.35, 0.7), mats.glass);
  cabin.position.set(0.25, 0.85, 0);
  root.add(cabin);

  // roll cage gold
  const cage = new THREE.Mesh(new THREE.TorusGeometry(0.42, 0.025, 8, 24, Math.PI), mats.gold);
  cage.rotation.z = Math.PI / 2;
  cage.position.set(0.15, 0.95, 0);
  root.add(cage);

  // bumper
  const bumper = new THREE.Mesh(new THREE.BoxGeometry(0.15, 0.18, 0.9), mats.graphite);
  bumper.position.set(0.75, 0.4, 0);
  root.add(bumper);
  const headL = new THREE.Mesh(new THREE.BoxGeometry(0.04, 0.1, 0.18), mats.emissive);
  headL.position.set(0.83, 0.45, 0.28);
  const headR = headL.clone();
  headR.position.z = -0.28;
  root.add(headL, headR);

  // wheels
  for (const [x, z] of [
    [-0.45, 0.48],
    [-0.45, -0.48],
    [0.45, 0.48],
    [0.45, -0.48],
  ]) {
    const wheel = new THREE.Group();
    const tire = new THREE.Mesh(new THREE.CylinderGeometry(0.28, 0.28, 0.18, 16), mats.rubber);
    tire.rotation.z = Math.PI / 2;
    const hub = new THREE.Mesh(new THREE.CylinderGeometry(0.12, 0.12, 0.2, 12), mats.graphite);
    hub.rotation.z = Math.PI / 2;
    const rim = new THREE.Mesh(new THREE.TorusGeometry(0.14, 0.02, 8, 20), mats.emissive);
    rim.rotation.y = Math.PI / 2;
    wheel.add(tire, hub, rim);
    wheel.position.set(x, 0.28, z);
    wheel.traverse((o) => {
      if (o.isMesh) {
        o.castShadow = true;
        o.receiveShadow = true;
      }
    });
    root.add(wheel);
  }

  // rear module plate
  const rear = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.35, 0.7), mats.ceramic);
  rear.position.set(-0.55, 0.65, 0);
  root.add(rear);
  const dish = new THREE.Mesh(new THREE.CylinderGeometry(0.18, 0.05, 0.08, 16), mats.graphite);
  dish.position.set(-0.55, 0.95, 0);
  root.add(dish);

  const light = new THREE.PointLight(0x3de0ff, 0.7, 8);
  light.position.set(0.8, 0.5, 0);
  root.add(light);

  const rMark = hullDecal('ELYSIUM  R-01', 0.7, 0.12);
  rMark.position.set(0.1, 0.78, 0.44);
  root.add(rMark);
  const rMark2 = hullDecal('EXPLORE · SURVEY · SUPPORT', 0.85, 0.1);
  rMark2.position.set(-0.2, 0.55, 0.44);
  root.add(rMark2);

  root.userData.kind = 'rover';
  root.userData.label = 'R-01 ROVER';
  return root;
}

/** Orbi Scout Drone — spherical recon. */
export function createOrbiScout(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'OrbiScout';

  const core = new THREE.Mesh(new THREE.SphereGeometry(0.38, 32, 24), mats.ceramic);
  core.castShadow = true;
  root.add(core);

  // panel belts
  const belt = new THREE.Mesh(new THREE.TorusGeometry(0.39, 0.03, 8, 32), mats.graphite);
  belt.rotation.x = Math.PI / 2;
  root.add(belt);
  const belt2 = belt.clone();
  belt2.rotation.x = 0;
  root.add(belt2);

  // sensor eye
  const socket = new THREE.Mesh(new THREE.CylinderGeometry(0.16, 0.18, 0.08, 24), mats.graphite);
  socket.rotation.x = Math.PI / 2;
  socket.position.z = 0.34;
  const lens = new THREE.Mesh(new THREE.SphereGeometry(0.13, 24, 24), mats.glass);
  lens.position.z = 0.38;
  const pupil = new THREE.Mesh(new THREE.CircleGeometry(0.05, 20), mats.emissive);
  pupil.position.z = 0.48;
  root.add(socket, lens, pupil);

  // top rotors / fins
  for (const x of [-0.22, 0.22]) {
    const fin = new THREE.Mesh(new THREE.BoxGeometry(0.08, 0.04, 0.28), mats.gold);
    fin.position.set(x, 0.35, 0);
    root.add(fin);
    const rotor = new THREE.Mesh(new THREE.CylinderGeometry(0.1, 0.1, 0.02, 12), mats.graphite);
    rotor.position.set(x, 0.4, 0);
    root.add(rotor);
  }

  // articulated stabilizer legs
  for (let i = 0; i < 4; i++) {
    const a = (i / 4) * Math.PI * 2 + Math.PI / 4;
    const limb = new THREE.Group();
    const u = new THREE.Mesh(new THREE.CapsuleGeometry(0.03, 0.18, 4, 6), mats.graphite);
    u.position.y = -0.2;
    const j = new THREE.Mesh(new THREE.SphereGeometry(0.04, 8, 8), mats.gold);
    j.position.y = -0.32;
    const tip = new THREE.Mesh(new THREE.SphereGeometry(0.035, 8, 8), mats.emissive);
    tip.position.y = -0.42;
    limb.add(u, j, tip);
    limb.position.set(Math.cos(a) * 0.28, 0, Math.sin(a) * 0.28);
    limb.rotation.z = Math.cos(a) * 0.5;
    limb.rotation.x = Math.sin(a) * 0.5;
    root.add(limb);
  }

  const light = new THREE.PointLight(0x3de0ff, 1.1, 7);
  light.position.set(0, 0, 0.5);
  root.add(light);

  // scan fan (concept: Scan mode holographic beam)
  const scanMat = mats.emissive.clone();
  scanMat.transparent = true;
  scanMat.opacity = 0.22;
  scanMat.depthWrite = false;
  scanMat.side = THREE.DoubleSide;
  const scan = new THREE.Mesh(new THREE.ConeGeometry(0.55, 1.1, 24, 1, true), scanMat);
  scan.rotation.x = Math.PI / 2;
  scan.position.set(0, 0, 0.95);
  scan.userData.pulse = true;
  root.add(scan);

  root.userData.kind = 'orbi_scout';
  root.userData.label = 'ORBI SCOUT';
  root.userData.hover = true;
  return root;
}

/** Hover Bike / Utility Sled. */
export function createHoverBike(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'HoverBike';

  const hull = armoredBox(1.6, 0.28, 0.55, mats, { goldRails: false });
  hull.position.y = 0.55;
  root.add(hull);

  // nose
  const nose = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.22, 0.5), mats.ceramic);
  nose.position.set(0.9, 0.55, 0);
  root.add(nose);
  for (const z of [-0.18, 0.18]) {
    const bar = new THREE.Mesh(new THREE.BoxGeometry(0.06, 0.22, 0.06), mats.emissive);
    bar.position.set(1.08, 0.55, z);
    root.add(bar);
  }

  // seat
  const seat = new THREE.Mesh(new THREE.BoxGeometry(0.45, 0.1, 0.35), mats.rubber);
  seat.position.set(-0.1, 0.75, 0);
  root.add(seat);

  // handlebars
  const bar = new THREE.Mesh(new THREE.CylinderGeometry(0.02, 0.02, 0.5, 8), mats.gold);
  bar.rotation.x = Math.PI / 2;
  bar.position.set(0.35, 0.85, 0);
  root.add(bar);
  const hud = new THREE.Mesh(new THREE.CircleGeometry(0.08, 16), mats.emissive);
  hud.position.set(0.35, 0.92, 0);
  hud.rotation.x = -0.5;
  root.add(hud);

  // hover pods
  for (const [x, z] of [
    [-0.45, 0.28],
    [-0.45, -0.28],
    [0.35, 0.28],
    [0.35, -0.28],
  ]) {
    const pod = new THREE.Mesh(new THREE.CylinderGeometry(0.14, 0.16, 0.12, 16), mats.graphite);
    pod.position.set(x, 0.28, z);
    const glow = new THREE.Mesh(new THREE.CircleGeometry(0.12, 16), mats.emissive);
    glow.rotation.x = -Math.PI / 2;
    glow.position.set(x, 0.21, z);
    root.add(pod, glow);
  }

  // rear cargo rack
  const rack = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.25, 0.45), mats.graphite);
  rack.position.set(-0.75, 0.7, 0);
  root.add(rack);

  const light = new THREE.PointLight(0x3de0ff, 0.8, 8);
  light.position.set(1.0, 0.4, 0);
  root.add(light);

  const hbMark = hullDecal('ELYSIUM  07', 0.55, 0.11);
  hbMark.position.set(0.2, 0.7, 0.29);
  root.add(hbMark);

  root.userData.kind = 'hover_bike';
  root.userData.label = 'HOVER BIKE';
  root.userData.hover = true;
  return root;
}

/** FAB-01 Field Fabricator. */
export function createFieldFabricator(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'FieldFabricator_FAB01';

  const body = armoredBox(1.1, 0.9, 1.0, mats, { goldRails: true });
  body.position.y = 0.85;
  root.add(body);

  // fabrication core window
  const core = new THREE.Mesh(new THREE.BoxGeometry(0.45, 0.45, 0.08), mats.glass);
  core.position.set(0, 0.95, 0.52);
  root.add(core);
  const coreGlow = new THREE.Mesh(new THREE.BoxGeometry(0.35, 0.35, 0.04), mats.emissive);
  coreGlow.position.set(0, 0.95, 0.48);
  root.add(coreGlow);

  // solar array
  const mast = new THREE.Mesh(new THREE.CylinderGeometry(0.03, 0.03, 0.5, 8), mats.graphite);
  mast.position.set(0, 1.55, 0);
  root.add(mast);
  for (const [x, z] of [
    [-0.35, -0.35],
    [0.35, -0.35],
    [-0.35, 0.35],
    [0.35, 0.35],
  ]) {
    const panel = new THREE.Mesh(new THREE.BoxGeometry(0.4, 0.02, 0.4), mats.graphite);
    panel.position.set(x, 1.75, z);
    panel.rotation.x = -0.35 * Math.sign(z || 1);
    panel.rotation.z = 0.25 * Math.sign(x || 1);
    const cell = new THREE.Mesh(new THREE.BoxGeometry(0.36, 0.01, 0.36), mats.emissive);
    cell.position.copy(panel.position);
    cell.position.y += 0.015;
    cell.rotation.copy(panel.rotation);
    root.add(panel, cell);
  }

  // control screen
  const screen = new THREE.Mesh(new THREE.BoxGeometry(0.08, 0.28, 0.35), mats.emissive);
  screen.position.set(0.58, 1.0, 0);
  root.add(screen);

  // output bay
  const bay = new THREE.Mesh(new THREE.BoxGeometry(0.5, 0.15, 0.35), mats.graphite);
  bay.position.set(0, 0.35, 0.55);
  root.add(bay);

  // retractable legs
  for (const [x, z] of [
    [-0.45, -0.45],
    [0.45, -0.45],
    [-0.45, 0.45],
    [0.45, 0.45],
  ]) {
    const hip = new THREE.Mesh(new THREE.SphereGeometry(0.06, 10, 10), mats.gold);
    hip.position.set(x, 0.45, z);
    const limb = new THREE.Mesh(new THREE.CapsuleGeometry(0.04, 0.28, 4, 8), mats.graphite);
    limb.position.set(x * 1.15, 0.22, z * 1.15);
    const foot = new THREE.Mesh(new THREE.CylinderGeometry(0.1, 0.12, 0.06, 10), mats.rubber);
    foot.position.set(x * 1.25, 0.05, z * 1.25);
    root.add(hip, limb, foot);
  }

  // antenna
  const ant = new THREE.Mesh(new THREE.CylinderGeometry(0.012, 0.012, 0.45, 6), mats.graphite);
  ant.position.set(0.3, 1.9, 0.2);
  const tip = new THREE.Mesh(new THREE.SphereGeometry(0.03, 8, 8), mats.emissive);
  tip.position.set(0.3, 2.15, 0.2);
  root.add(ant, tip);

  const light = new THREE.PointLight(0x3de0ff, 1.2, 10);
  light.position.set(0, 1.0, 0.6);
  root.add(light);

  const fMark = hullDecal('FAB-01  CONSTRUCT', 0.7, 0.12);
  fMark.position.set(0, 1.15, 0.52);
  root.add(fMark);
  const fMark2 = hullDecal('ELYSIUM', 0.45, 0.1);
  fMark2.position.set(-0.56, 1.0, 0);
  fMark2.rotation.y = -Math.PI / 2;
  root.add(fMark2);

  root.userData.kind = 'fabricator';
  root.userData.label = 'FAB-01 FIELD FABRICATOR';
  return root;
}

export const HERO_BUILDERS = {
  terra_drone: createTerraDrone,
  rover: createRover,
  orbi_scout: createOrbiScout,
  hover_bike: createHoverBike,
  fabricator: createFieldFabricator,
};
