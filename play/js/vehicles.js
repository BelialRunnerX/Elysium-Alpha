/**
 * Elysium surface vehicles — Fourth Edition Part 16.4 + concept sheets.
 *
 * Catalogue mirrors the attached production sheets:
 *   TDR-01 Terra-Drone · R-01 Rover · Orbi Scout · Hover Bike
 * Modes are gameplay verbs from those sheets (Idle/Build/Plant/Carry, etc.).
 */

export const VEHICLE_CATALOGUE = {
  terra_drone: {
    id: 'terra_drone',
    codename: 'TDR-01',
    label: 'TERRA-DRONE',
    role: 'Utility / Support',
    pdfClass: 'Mining Rig / field utility',
    rideable: true,
    driveSpeed: 9.5,
    sprintSpeed: 13,
    energyDrain: 2.2,
    hover: false,
    seatOffset: [0, 1.15, -0.15],
    modes: [
      { id: 'idle', name: 'IDLE / SCAN', verb: 'Standby scan pulse' },
      { id: 'build', name: 'BUILD / REPAIR', verb: 'Holo-construct assist' },
      { id: 'plant', name: 'PLANT SEED', verb: 'Ecological assist' },
      { id: 'carry', name: 'CARRY / DELIVER', verb: 'Back-rack logistics' },
    ],
  },
  rover: {
    id: 'rover',
    codename: 'R-01',
    label: 'ROVER',
    role: 'Compact Ground Vehicle',
    pdfClass: 'Scout Rover',
    rideable: true,
    driveSpeed: 14,
    sprintSpeed: 20,
    energyDrain: 3.4,
    hover: false,
    seatOffset: [0, 1.35, 0.1],
    modes: [
      { id: 'research', name: 'RESEARCH', verb: 'Sensors · Analysis · Discovery' },
      { id: 'cargo', name: 'CARGO', verb: 'Haul · Supply · Expedition' },
      { id: 'utility', name: 'UTILITY', verb: 'Field work · Maintenance' },
      { id: 'excavate', name: 'EXCAVATION', verb: 'Excavate · Collect · Process' },
    ],
  },
  orbi_scout: {
    id: 'orbi_scout',
    codename: 'ORBI',
    label: 'ORBI SCOUT',
    role: 'Primary Scout Drone',
    pdfClass: 'Survey relay / escort drone',
    rideable: false,
    driveSpeed: 11,
    sprintSpeed: 16,
    energyDrain: 1.6,
    hover: true,
    seatOffset: [0, 1.4, 0],
    modes: [
      { id: 'hover', name: 'HOVER', verb: 'Station keep / idle' },
      { id: 'scan', name: 'SCAN', verb: 'Environment hologram' },
      { id: 'follow', name: 'FOLLOW', verb: 'Ally assist / autonomous' },
      { id: 'assist', name: 'ASSIST', verb: 'Carry / deliver / utility' },
    ],
  },
  hover_bike: {
    id: 'hover_bike',
    codename: 'HB-07',
    label: 'HOVER BIKE',
    role: 'Utility Sled',
    pdfClass: 'Hover Sled',
    rideable: true,
    driveSpeed: 18,
    sprintSpeed: 28,
    energyDrain: 5.5,
    hover: true,
    seatOffset: [0, 1.05, -0.2],
    modes: [
      { id: 'explore', name: 'EXPLORATION', verb: 'Fast personal travel' },
      { id: 'cargo', name: 'CARGO HAULER', verb: 'Logistics · supply rack' },
      { id: 'survey', name: 'SURVEY / SCOUT', verb: 'Mapping · sensor mast' },
    ],
  },
  fabricator: {
    id: 'fabricator',
    codename: 'FAB-01',
    label: 'FIELD FABRICATOR',
    role: 'Stationary construct',
    pdfClass: 'Base machine',
    rideable: false,
    driveSpeed: 0,
    sprintSpeed: 0,
    energyDrain: 0,
    hover: false,
    seatOffset: [0, 0, 0],
    modes: [
      { id: 'print', name: 'FABRICATE', verb: 'Field print · +steel' },
    ],
  },
};

export function catalogueFor(kind) {
  return VEHICLE_CATALOGUE[kind] || null;
}

export function modeLabel(spec, modeIndex = 0) {
  if (!spec?.modes?.length) return 'STANDBY';
  const m = spec.modes[((modeIndex % spec.modes.length) + spec.modes.length) % spec.modes.length];
  return m.name;
}

export function cycleMode(spec, modeIndex = 0, delta = 1) {
  if (!spec?.modes?.length) return 0;
  return ((modeIndex + delta) % spec.modes.length + spec.modes.length) % spec.modes.length;
}
