# Surface vehicles (Fourth Edition Part 16.4)

Playable reference lives in the browser client (`play/js/vehicles.js` + hero meshes).
Minecraft entity implementations can land in a later pass; this document locks the
catalogue so NeoForge content matches the concept sheets and the C++ registry.

| Sheet id | Codename | PDF class | Rideable | Modes |
|---|---|---|---|---|
| `terra_drone` | TDR-01 | Mining Rig / field utility | yes | Idle/Scan · Build/Repair · Plant Seed · Carry/Deliver |
| `rover` | R-01 | Scout Rover | yes | Research · Cargo · Utility · Excavation |
| `orbi_scout` | ORBI | Survey relay / escort | no (escort) | Hover · Scan · Follow · Assist |
| `hover_bike` | HB-07 | Hover Sled | yes | Exploration · Cargo Hauler · Survey/Scout |

## Controls (browser vertical slice)

- **E** board / assign Orbi escort / fabricate
- **Q** dismount
- **R** cycle function mode
- **F** activate mode (or melee when on foot)

Industrial modes raise Suspicion (Empire attention), matching Part 16 logistics pressure.
