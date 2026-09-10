/**
 * Hero hard-surface assets — Elysium concept sheet fidelity pass.
 * Anatomy + palette locked to play/assets/concepts/*.jpg
 */
import {
  elysiumMaterials,
  armoredBox,
  boltRing,
  goldJoint,
  hullDecal,
  statusStrip,
  armorPlate,
} from '../materials.js';

function shadowize(root) {
  root.traverse((o) => {
    if (o.isMesh) {
      o.castShadow = true;
      o.receiveShadow = true;
    }
  });
}

/** Chunky quadruped leg — white armor over graphite with gold joint caps (sheet). */
function terraLeg(THREE, mats, sideX, sideZ) {
  const g = new THREE.Group();
  const hip = goldJoint(0.07, mats);
  hip.position.set(0, 0, 0);

  const upperArmor = armorPlate(0.11, 0.22, 0.14, mats);
  upperArmor.position.set(sideX * 0.04, -0.12, sideZ * 0.02);
  upperArmor.rotation.z = sideX * 0.28;
  upperArmor.rotation.x = sideZ * 0.15;

  const upperBone = new THREE.Mesh(
    new THREE.CapsuleGeometry(0.035, 0.16, 4, 8),
    mats.graphite,
  );
  upperBone.position.copy(upperArmor.position);

  const knee = goldJoint(0.065, mats);
  knee.position.set(sideX * 0.1, -0.28, sideZ * 0.06);
  const kneeGlow = new THREE.Mesh(
    new THREE.TorusGeometry(0.055, 0.012, 8, 18),
    mats.emissive.clone(),
  );
  kneeGlow.position.copy(knee.position);
  kneeGlow.rotation.x = Math.PI / 2;
  kneeGlow.userData.pulse = true;

  const lowerArmor = armorPlate(0.1, 0.2, 0.12, mats);
  lowerArmor.position.set(sideX * 0.14, -0.42, sideZ * 0.09);
  lowerArmor.rotation.z = -sideX * 0.2;

  const lowerBone = new THREE.Mesh(
    new THREE.CapsuleGeometry(0.03, 0.14, 4, 8),
    mats.graphite,
  );
  lowerBone.position.copy(lowerArmor.position);

  const ankle = goldJoint(0.05, mats);
  ankle.position.set(sideX * 0.16, -0.54, sideZ * 0.1);

  const foot = new THREE.Mesh(new THREE.BoxGeometry(0.16, 0.06, 0.22), mats.rubber);
  foot.position.set(sideX * 0.16, -0.6, sideZ * 0.12);
  const footPad = new THREE.Mesh(new THREE.BoxGeometry(0.14, 0.02, 0.18), mats.graphite);
  footPad.position.set(sideX * 0.16, -0.635, sideZ * 0.12);

  g.add(hip, upperBone, upperArmor, knee, kneeGlow, lowerBone, lowerArmor, ankle, foot, footPad);
  g.position.set(sideX * 0.3, 0.62, sideZ * 0.28);
  shadowize(g);
  return g;
}

function manipulator(THREE, mats) {
  const arm = new THREE.Group();
  const shoulder = goldJoint(0.055, mats);
  const bone1 = new THREE.Mesh(new THREE.CapsuleGeometry(0.032, 0.2, 4, 8), mats.graphite);
  bone1.position.set(0.14, -0.02, 0.04);
  bone1.rotation.z = -0.45;
  bone1.rotation.y = 0.35;
  const elbow = goldJoint(0.045, mats);
  elbow.position.set(0.26, -0.12, 0.1);
  const bone2 = new THREE.Mesh(new THREE.CapsuleGeometry(0.028, 0.16, 4, 8), mats.graphite);
  bone2.position.set(0.34, -0.2, 0.16);
  bone2.rotation.z = -0.75;
  const wrist = new THREE.Mesh(new THREE.CylinderGeometry(0.04, 0.04, 0.06, 10), mats.ceramic);
  wrist.position.set(0.4, -0.28, 0.22);
  const claw = new THREE.Group();
  for (let i = 0; i < 3; i++) {
    const a = (i / 3) * Math.PI * 2 - Math.PI / 2;
    const finger = new THREE.Mesh(new THREE.BoxGeometry(0.022, 0.1, 0.028), mats.graphite);
    finger.position.set(Math.cos(a) * 0.045, -0.06, Math.sin(a) * 0.045);
    finger.rotation.x = 0.55;
    const tip = new THREE.Mesh(new THREE.SphereGeometry(0.014, 8, 8), mats.emissive.clone());
    tip.position.set(0, -0.05, 0);
    tip.userData.pulse = true;
    finger.add(tip);
    claw.add(finger);
  }
  claw.position.copy(wrist.position);
  claw.position.y -= 0.04;
  arm.add(shoulder, bone1, elbow, bone2, wrist, claw);
  arm.position.set(0.36, 0.38, 0.34);
  shadowize(arm);
  return arm;
}

/** Orbi wing/fin pod from the scout sheet — ceramic shell + cyan strip + gold hinge. */
function orbiPod(THREE, mats, x, y, z, rotZ = 0, rotX = 0) {
  const g = new THREE.Group();
  const hinge = goldJoint(0.04, mats);
  const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.025, 0.16, 4, 8), mats.graphite);
  arm.position.set(0.12, 0, 0);
  arm.rotation.z = -0.35;
  const shell = new THREE.Mesh(new THREE.CapsuleGeometry(0.07, 0.18, 6, 12), mats.ceramic);
  shell.position.set(0.28, 0, 0);
  shell.rotation.z = Math.PI / 2;
  const strip = new THREE.Mesh(new THREE.BoxGeometry(0.2, 0.02, 0.035), mats.emissive.clone());
  strip.position.set(0.28, 0.05, 0);
  strip.userData.pulse = true;
  const tip = new THREE.Mesh(new THREE.SphereGeometry(0.035, 10, 10), mats.gold);
  tip.position.set(0.42, 0, 0);
  g.add(hinge, arm, shell, strip, tip);
  g.position.set(x, y, z);
  g.rotation.z = rotZ;
  g.rotation.x = rotX;
  shadowize(g);
  return g;
}

/** TDR-01 Terra-Drone — quadruped utility / support (sheet match). */
export function createTerraDrone(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'TerraDrone_TDR01';

  const chassis = armoredBox(0.72, 0.34, 0.58, mats, { goldRails: true, sideVents: true });
  chassis.position.y = 0.64;
  root.add(chassis);

  // top solar tiles (sheet)
  for (const [x, z] of [
    [-0.18, -0.08],
    [0.18, -0.08],
    [-0.18, 0.12],
    [0.18, 0.12],
  ]) {
    const tile = new THREE.Mesh(new THREE.BoxGeometry(0.22, 0.015, 0.16), mats.graphite);
    tile.position.set(x, 0.82, z);
    const cell = new THREE.Mesh(new THREE.BoxGeometry(0.18, 0.008, 0.12), mats.emissive.clone());
    cell.position.set(x, 0.83, z);
    cell.userData.pulse = true;
    root.add(tile, cell);
  }

  // concentric sensor eye
  const eyeHousing = new THREE.Mesh(new THREE.CylinderGeometry(0.13, 0.15, 0.09, 24), mats.graphite);
  eyeHousing.rotation.x = Math.PI / 2;
  eyeHousing.position.set(0, 0.64, 0.32);
  const ringGold = new THREE.Mesh(new THREE.TorusGeometry(0.12, 0.015, 8, 24), mats.gold);
  ringGold.position.set(0, 0.64, 0.36);
  const eye = new THREE.Mesh(new THREE.SphereGeometry(0.095, 24, 24), mats.glass);
  eye.position.set(0, 0.64, 0.36);
  const iris = new THREE.Mesh(new THREE.CircleGeometry(0.045, 24), mats.emissive.clone());
  iris.position.set(0, 0.64, 0.42);
  iris.userData.pulse = true;
  const bolts = boltRing(0.125, mats, 8);
  bolts.position.set(0, 0.64, 0.32);
  bolts.rotation.x = Math.PI / 2;
  root.add(eyeHousing, ringGold, eye, iris, bolts);

  // status strips
  for (const x of [-0.3, 0.3]) {
    const strip = statusStrip(0.38, mats);
    strip.rotation.y = Math.PI / 2;
    strip.scale.set(1, 2.4, 1);
    strip.position.set(x, 0.72, 0);
    root.add(strip);
  }

  const mark = hullDecal('ELYSIUM  TDR-01', 0.58, 0.11);
  mark.position.set(0, 0.74, 0.3);
  root.add(mark);
  const markSide = hullDecal('SUPPORT', 0.3, 0.08);
  markSide.position.set(0.375, 0.64, 0);
  markSide.rotation.y = Math.PI / 2;
  root.add(markSide);

  // service ports
  for (const z of [-0.14, 0.14]) {
    const port = new THREE.Mesh(new THREE.CylinderGeometry(0.032, 0.032, 0.025, 10), mats.graphite);
    port.rotation.x = Math.PI / 2;
    port.position.set(-0.37, 0.64, z);
    root.add(port);
  }

  // antenna mast
  const ant = new THREE.Mesh(new THREE.CylinderGeometry(0.01, 0.01, 0.3, 6), mats.graphite);
  ant.position.set(-0.22, 0.95, -0.12);
  const antTip = new THREE.Mesh(new THREE.SphereGeometry(0.028, 8, 8), mats.emissive.clone());
  antTip.position.set(-0.22, 1.1, -0.12);
  antTip.userData.pulse = true;
  root.add(ant, antTip);

  root.add(terraLeg(THREE, mats, -1, 1));
  root.add(terraLeg(THREE, mats, 1, 1));
  root.add(terraLeg(THREE, mats, -1, -1));
  root.add(terraLeg(THREE, mats, 1, -1));
  root.add(manipulator(THREE, mats));

  // rear vent module
  const vent = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.14, 0.1), mats.graphite);
  vent.position.set(0, 0.64, -0.32);
  for (let i = 0; i < 5; i++) {
    const slat = new THREE.Mesh(new THREE.BoxGeometry(0.3, 0.012, 0.02), mats.emissive.clone());
    slat.position.set(0, 0.58 + i * 0.025, -0.37);
    slat.userData.pulse = true;
    root.add(slat);
  }
  root.add(vent);

  const light = new THREE.PointLight(0x3de0ff, 1.15, 7);
  light.position.set(0, 0.7, 0.45);
  root.add(light);

  // build/repair holo (function mode)
  const gridMat = mats.emissive.clone();
  gridMat.transparent = true;
  gridMat.opacity = 0.28;
  gridMat.depthWrite = false;
  const grid = new THREE.GridHelper(1.1, 10, 0x3de0ff, 0x1a6a80);
  grid.position.set(0.6, 0.02, 0.55);
  root.add(grid);
  const holo = new THREE.Mesh(new THREE.BoxGeometry(0.32, 0.32, 0.32), gridMat);
  holo.position.set(0.6, 0.24, 0.55);
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

  const body = armoredBox(1.4, 0.48, 0.9, mats, { goldRails: true, sideVents: true });
  body.position.y = 0.58;
  root.add(body);

  const cabin = new THREE.Mesh(new THREE.BoxGeometry(0.58, 0.36, 0.72), mats.glass);
  cabin.position.set(0.28, 0.9, 0);
  root.add(cabin);

  // gold roll cage
  const cage = new THREE.Mesh(new THREE.TorusGeometry(0.44, 0.028, 8, 28, Math.PI), mats.gold);
  cage.rotation.z = Math.PI / 2;
  cage.position.set(0.18, 1.0, 0);
  root.add(cage);
  for (const z of [-0.32, 0.32]) {
    const bar = new THREE.Mesh(new THREE.CapsuleGeometry(0.02, 0.35, 4, 8), mats.gold);
    bar.position.set(0.05, 1.05, z);
    root.add(bar);
  }

  const bumper = new THREE.Mesh(new THREE.BoxGeometry(0.16, 0.2, 0.95), mats.graphite);
  bumper.position.set(0.78, 0.42, 0);
  root.add(bumper);
  for (const z of [-0.3, 0.3]) {
    const head = new THREE.Mesh(new THREE.BoxGeometry(0.05, 0.12, 0.2), mats.emissive.clone());
    head.position.set(0.87, 0.48, z);
    head.userData.pulse = true;
    root.add(head);
  }
  // winch / sensor on bumper
  const winch = new THREE.Mesh(new THREE.CylinderGeometry(0.06, 0.06, 0.14, 12), mats.gold);
  winch.rotation.z = Math.PI / 2;
  winch.position.set(0.86, 0.35, 0);
  root.add(winch);

  // suspension + wheels
  for (const [x, z] of [
    [-0.48, 0.5],
    [-0.48, -0.5],
    [0.48, 0.5],
    [0.48, -0.5],
  ]) {
    const wheel = new THREE.Group();
    const tire = new THREE.Mesh(new THREE.CylinderGeometry(0.3, 0.3, 0.2, 18), mats.rubber);
    tire.rotation.z = Math.PI / 2;
    const hub = new THREE.Mesh(new THREE.CylinderGeometry(0.13, 0.13, 0.22, 14), mats.graphite);
    hub.rotation.z = Math.PI / 2;
    const rim = new THREE.Mesh(new THREE.TorusGeometry(0.15, 0.022, 8, 22), mats.emissive.clone());
    rim.rotation.y = Math.PI / 2;
    rim.userData.pulse = true;
    // coil-over
    const shock = new THREE.Mesh(new THREE.CylinderGeometry(0.03, 0.03, 0.28, 8), mats.emissive.clone());
    shock.position.set(0, 0.28, 0);
    shock.userData.pulse = true;
    const arm = new THREE.Mesh(new THREE.BoxGeometry(0.08, 0.04, 0.25), mats.graphite);
    arm.position.set(0, 0.18, 0);
    wheel.add(tire, hub, rim, shock, arm);
    wheel.position.set(x, 0.3, z);
    shadowize(wheel);
    root.add(wheel);
  }

  // research dish / cargo plate
  const rear = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.38, 0.75), mats.ceramic);
  rear.position.set(-0.58, 0.7, 0);
  root.add(rear);
  const dish = new THREE.Mesh(new THREE.CylinderGeometry(0.2, 0.06, 0.09, 18), mats.graphite);
  dish.position.set(-0.58, 1.0, 0);
  const dishRing = new THREE.Mesh(new THREE.TorusGeometry(0.18, 0.015, 8, 20), mats.gold);
  dishRing.rotation.x = Math.PI / 2;
  dishRing.position.set(-0.58, 1.02, 0);
  root.add(dish, dishRing);

  const light = new THREE.PointLight(0x3de0ff, 0.9, 9);
  light.position.set(0.85, 0.55, 0);
  root.add(light);

  const rMark = hullDecal('ELYSIUM  R-01', 0.72, 0.12);
  rMark.position.set(0.12, 0.82, 0.46);
  root.add(rMark);
  const rMark2 = hullDecal('EXPLORE · SURVEY · SUPPORT', 0.9, 0.1);
  rMark2.position.set(-0.15, 0.58, 0.46);
  root.add(rMark2);

  root.userData.kind = 'rover';
  root.userData.label = 'R-01 ROVER';
  return root;
}

/** Orbi Scout — spherical recon with four articulated fin pods (sheet match). */
export function createOrbiScout(THREE) {
  const mats = elysiumMaterials();
  const root = new THREE.Group();
  root.name = 'OrbiScout';

  const core = new THREE.Mesh(new THREE.SphereGeometry(0.4, 40, 28), mats.ceramic);
  core.castShadow = true;
  root.add(core);

  // panel belts + gold meridian rings
  const belt = new THREE.Mesh(new THREE.TorusGeometry(0.41, 0.028, 8, 40), mats.graphite);
  belt.rotation.x = Math.PI / 2;
  root.add(belt);
  const beltGold = new THREE.Mesh(new THREE.TorusGeometry(0.405, 0.012, 8, 40), mats.gold);
  beltGold.rotation.x = Math.PI / 2;
  root.add(beltGold);
  const belt2 = belt.clone();
  belt2.rotation.x = 0;
  root.add(belt2);

  // multi-ring sensor eye
  const socket = new THREE.Mesh(new THREE.CylinderGeometry(0.17, 0.19, 0.09, 28), mats.graphite);
  socket.rotation.x = Math.PI / 2;
  socket.position.z = 0.36;
  const goldEye = new THREE.Mesh(new THREE.TorusGeometry(0.15, 0.018, 8, 28), mats.gold);
  goldEye.position.z = 0.4;
  const lens = new THREE.Mesh(new THREE.SphereGeometry(0.135, 28, 28), mats.glass);
  lens.position.z = 0.4;
  const pupil = new THREE.Mesh(new THREE.CircleGeometry(0.055, 24), mats.emissive.clone());
  pupil.position.z = 0.5;
  pupil.userData.pulse = true;
  const eyeBolts = boltRing(0.16, mats, 10);
  eyeBolts.position.z = 0.36;
  eyeBolts.rotation.x = Math.PI / 2;
  root.add(socket, goldEye, lens, pupil, eyeBolts);

  // four articulated fin pods (upper rear + lower sides)
  root.add(orbiPod(THREE, mats, 0.22, 0.22, -0.15, -0.55, 0.2));
  root.add(orbiPod(THREE, mats, -0.22, 0.22, -0.15, 0.55 + Math.PI, -0.2));
  root.add(orbiPod(THREE, mats, 0.32, -0.12, 0.05, -0.9, 0.15));
  root.add(orbiPod(THREE, mats, -0.32, -0.12, 0.05, 0.9 + Math.PI, -0.15));

  // underside port
  const under = new THREE.Mesh(new THREE.BoxGeometry(0.16, 0.04, 0.1), mats.emissive.clone());
  under.position.set(0, -0.38, 0);
  under.userData.pulse = true;
  root.add(under);

  // rear exhaust
  const exhaust = new THREE.Mesh(new THREE.CylinderGeometry(0.08, 0.1, 0.06, 14), mats.graphite);
  exhaust.rotation.x = Math.PI / 2;
  exhaust.position.set(0, 0, -0.4);
  const exhaustGlow = new THREE.Mesh(new THREE.CircleGeometry(0.06, 16), mats.emissive.clone());
  exhaustGlow.position.set(0, 0, -0.44);
  exhaustGlow.userData.pulse = true;
  root.add(exhaust, exhaustGlow);

  const mark = hullDecal('ELYSIUM', 0.35, 0.08);
  mark.position.set(0, 0.28, 0.28);
  mark.rotation.x = -0.6;
  root.add(mark);
  const mark2 = hullDecal('RECON · SCAN · SUPPORT', 0.45, 0.08);
  mark2.position.set(0, -0.1, 0.38);
  root.add(mark2);

  const light = new THREE.PointLight(0x3de0ff, 1.4, 8);
  light.position.set(0, 0, 0.55);
  root.add(light);

  // scan fan
  const scanMat = mats.emissive.clone();
  scanMat.transparent = true;
  scanMat.opacity = 0.2;
  scanMat.depthWrite = false;
  scanMat.side = THREE.DoubleSide;
  const scan = new THREE.Mesh(new THREE.ConeGeometry(0.65, 1.2, 28, 1, true), scanMat);
  scan.rotation.x = Math.PI / 2;
  scan.position.set(0, 0, 1.05);
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

  const hull = armoredBox(1.65, 0.3, 0.58, mats, { goldRails: false, sideVents: true });
  hull.position.y = 0.58;
  root.add(hull);

  // protective gold rails
  for (const z of [-0.32, 0.32]) {
    const rail = new THREE.Mesh(new THREE.CapsuleGeometry(0.025, 1.2, 4, 8), mats.gold);
    rail.rotation.z = Math.PI / 2;
    rail.position.set(0.1, 0.72, z);
    root.add(rail);
  }

  const nose = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.24, 0.52), mats.ceramic);
  nose.position.set(0.95, 0.58, 0);
  root.add(nose);
  for (const z of [-0.18, 0.18]) {
    const bar = new THREE.Mesh(new THREE.BoxGeometry(0.07, 0.24, 0.07), mats.emissive.clone());
    bar.position.set(1.15, 0.58, z);
    bar.userData.pulse = true;
    root.add(bar);
  }
  // intake
  const intake = new THREE.Mesh(new THREE.CylinderGeometry(0.1, 0.12, 0.08, 14), mats.graphite);
  intake.rotation.z = Math.PI / 2;
  intake.position.set(1.12, 0.58, 0);
  root.add(intake);

  const seat = new THREE.Mesh(new THREE.BoxGeometry(0.48, 0.1, 0.36), mats.rubber);
  seat.position.set(-0.08, 0.78, 0);
  root.add(seat);
  const backrest = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.28, 0.34), mats.rubber);
  backrest.position.set(-0.32, 0.9, 0);
  root.add(backrest);

  const bar = new THREE.Mesh(new THREE.CylinderGeometry(0.022, 0.022, 0.52, 8), mats.gold);
  bar.rotation.x = Math.PI / 2;
  bar.position.set(0.38, 0.9, 0);
  root.add(bar);
  const grips = [
    [-0.22, 0.9],
    [0.22, 0.9],
  ];
  for (const [gz] of [[-0.22], [0.22]]) {
    const grip = new THREE.Mesh(new THREE.CylinderGeometry(0.03, 0.03, 0.08, 8), mats.rubber);
    grip.rotation.x = Math.PI / 2;
    grip.position.set(0.38, 0.9, gz);
    root.add(grip);
  }
  const hud = new THREE.Mesh(new THREE.CircleGeometry(0.09, 18), mats.emissive.clone());
  hud.position.set(0.38, 0.98, 0);
  hud.rotation.x = -0.55;
  hud.userData.pulse = true;
  root.add(hud);

  // hover pods with gold trim
  for (const [x, z] of [
    [-0.5, 0.3],
    [-0.5, -0.3],
    [0.4, 0.3],
    [0.4, -0.3],
  ]) {
    const pod = new THREE.Mesh(new THREE.CylinderGeometry(0.15, 0.17, 0.14, 18), mats.graphite);
    pod.position.set(x, 0.3, z);
    const trim = new THREE.Mesh(new THREE.TorusGeometry(0.14, 0.018, 8, 20), mats.gold);
    trim.rotation.x = Math.PI / 2;
    trim.position.set(x, 0.3, z);
    const glow = new THREE.Mesh(new THREE.CircleGeometry(0.13, 18), mats.emissive.clone());
    glow.rotation.x = -Math.PI / 2;
    glow.position.set(x, 0.22, z);
    glow.userData.pulse = true;
    root.add(pod, trim, glow);
  }

  const rack = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.28, 0.48), mats.graphite);
  rack.position.set(-0.8, 0.75, 0);
  root.add(rack);
  const crate = new THREE.Mesh(new THREE.BoxGeometry(0.28, 0.22, 0.35), mats.ceramic);
  crate.position.set(-0.8, 0.95, 0);
  root.add(crate);

  // rear brake bar
  const brake = new THREE.Mesh(new THREE.BoxGeometry(0.08, 0.06, 0.5), mats.emissive.clone());
  brake.position.set(-1.0, 0.55, 0);
  brake.material = mats.emissive.clone();
  brake.material.emissive = new THREE.Color(0xff3355);
  brake.material.emissiveIntensity = 2.2;
  root.add(brake);

  const light = new THREE.PointLight(0x3de0ff, 1.0, 9);
  light.position.set(1.05, 0.45, 0);
  root.add(light);

  const hbMark = hullDecal('ELYSIUM  07', 0.58, 0.11);
  hbMark.position.set(0.25, 0.74, 0.3);
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

  const body = armoredBox(1.15, 0.95, 1.05, mats, { goldRails: true, sideVents: true });
  body.position.y = 0.9;
  root.add(body);

  const core = new THREE.Mesh(new THREE.BoxGeometry(0.48, 0.48, 0.1), mats.glass);
  core.position.set(0, 1.0, 0.55);
  root.add(core);
  const coreGlow = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.38, 0.05), mats.emissive.clone());
  coreGlow.position.set(0, 1.0, 0.5);
  coreGlow.userData.pulse = true;
  root.add(coreGlow);
  // inner greebles visible through glass
  for (let i = 0; i < 6; i++) {
    const cog = new THREE.Mesh(new THREE.TorusGeometry(0.06 + i * 0.02, 0.01, 6, 14), mats.gold);
    cog.position.set((i % 2) * 0.1 - 0.05, 1.0, 0.48);
    cog.rotation.y = i * 0.4;
    root.add(cog);
  }

  const mast = new THREE.Mesh(new THREE.CylinderGeometry(0.035, 0.035, 0.55, 8), mats.graphite);
  mast.position.set(0, 1.65, 0);
  root.add(mast);
  for (const [x, z] of [
    [-0.38, -0.38],
    [0.38, -0.38],
    [-0.38, 0.38],
    [0.38, 0.38],
  ]) {
    const panel = new THREE.Mesh(new THREE.BoxGeometry(0.42, 0.025, 0.42), mats.graphite);
    panel.position.set(x, 1.85, z);
    panel.rotation.x = -0.4 * Math.sign(z || 1);
    panel.rotation.z = 0.28 * Math.sign(x || 1);
    const cell = new THREE.Mesh(new THREE.BoxGeometry(0.36, 0.012, 0.36), mats.emissive.clone());
    cell.position.copy(panel.position);
    cell.position.y += 0.018;
    cell.rotation.copy(panel.rotation);
    cell.userData.pulse = true;
    root.add(panel, cell);
  }

  const screen = new THREE.Mesh(new THREE.BoxGeometry(0.09, 0.3, 0.38), mats.emissive.clone());
  screen.position.set(0.6, 1.05, 0);
  screen.userData.pulse = true;
  root.add(screen);
  const screenFrame = new THREE.Mesh(new THREE.BoxGeometry(0.04, 0.34, 0.42), mats.gold);
  screenFrame.position.set(0.56, 1.05, 0);
  root.add(screenFrame);

  const bay = new THREE.Mesh(new THREE.BoxGeometry(0.55, 0.16, 0.38), mats.graphite);
  bay.position.set(0, 0.38, 0.58);
  root.add(bay);
  const bayGlow = new THREE.Mesh(new THREE.BoxGeometry(0.45, 0.04, 0.02), mats.emissive.clone());
  bayGlow.position.set(0, 0.38, 0.78);
  bayGlow.userData.pulse = true;
  root.add(bayGlow);

  for (const [x, z] of [
    [-0.48, -0.48],
    [0.48, -0.48],
    [-0.48, 0.48],
    [0.48, 0.48],
  ]) {
    const hip = goldJoint(0.065, mats);
    hip.position.set(x, 0.48, z);
    const limb = new THREE.Mesh(new THREE.CapsuleGeometry(0.045, 0.3, 4, 8), mats.graphite);
    limb.position.set(x * 1.18, 0.24, z * 1.18);
    const foot = new THREE.Mesh(new THREE.CylinderGeometry(0.11, 0.13, 0.07, 12), mats.rubber);
    foot.position.set(x * 1.28, 0.05, z * 1.28);
    const footArmor = armorPlate(0.14, 0.04, 0.14, mats);
    footArmor.position.set(x * 1.28, 0.1, z * 1.28);
    root.add(hip, limb, foot, footArmor);
  }

  const ant = new THREE.Mesh(new THREE.CylinderGeometry(0.012, 0.012, 0.5, 6), mats.graphite);
  ant.position.set(0.32, 2.0, 0.22);
  const tip = new THREE.Mesh(new THREE.SphereGeometry(0.032, 8, 8), mats.emissive.clone());
  tip.position.set(0.32, 2.28, 0.22);
  tip.userData.pulse = true;
  root.add(ant, tip);

  const light = new THREE.PointLight(0x3de0ff, 1.5, 11);
  light.position.set(0, 1.05, 0.65);
  root.add(light);

  const fMark = hullDecal('FAB-01  CONSTRUCT', 0.72, 0.12);
  fMark.position.set(0, 1.22, 0.54);
  root.add(fMark);
  const fMark2 = hullDecal('ELYSIUM', 0.48, 0.1);
  fMark2.position.set(-0.59, 1.05, 0);
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
