# Elysium Art Bible — Hero Asset Quality Bar

**Authority:** Concept sheets in `play/assets/concepts/` and `docs/art/concepts/`.

## Palette (non-negotiable)

| Token | Hex | Use |
|---|---|---|
| Ceramic Composite (Suit White) | `#F4F6F8` | Outer shells, panels |
| Graphite Metal | `#2C3036` | Frames, joints, undersides |
| Solar-Gold Accent | `#C9A227` | Rails, joint caps, trim |
| Emissive Cyan | `#3DE0FF` | Sensors, status, hover glow |
| Anti-Slip Rubber | `#1A1A1A` | Seats, feet, tires |

## Hero roster

| ID | Asset | Concept file | In-game builder |
|---|---|---|---|
| TDR-01 | Terra-Drone (utility) | `terra_drone_tdr01.jpg` | `createTerraDrone` |
| R-01 | Rover | `rover_r01.jpg` | `createRover` |
| ORBI | Orbi Scout Drone | `orbi_scout_drone.jpg` | `createOrbiScout` |
| HB-07 | Hover Bike / Utility Sled | `hover_bike_utility.jpg` | `createHoverBike` |
| FAB-01 | Field Fabricator | `field_fabricator_fab01.jpg` | `createFieldFabricator` |

## Quality rules

1. **Hard-surface readability** — panel breaks, bolt rings, gold joint caps, graphite mechanicals under ceramic shells.
2. **Emissive presence** — every hero carries at least one cyan light + PointLight for night readability.
3. **Modularity** — builders expose `userData.kind` / `userData.label` for interaction and future loadout swaps.
4. **Shadows** — cast + receive on primary meshes.
5. **Scale** — drones ~0.8–1.0 m; rover ~1.35 m long; fabricator ~1.6 m; hover bike ~1.6 m.

## Runtime

Materials: `play/js/materials.js` (`elysiumMaterials()`).
Meshes: `play/js/assets/heroes.js`.

Next fidelity steps: bake high-poly → GLB with Substance-style maps, LODs, and decal atlases (ELYSIUM / unit codes) matching the sheets.


## Fidelity ceiling (current vs sheets)

Procedural Three.js builders approximate the sheets (palette, silhouette, emissives, hull codes, modular hooks). They are **not** yet Substance-baked hero GLBs.

**Next fidelity step:** high-poly sculpt → bake normals/AO/curvature → GLB + LODs + decal atlas (ELYSIUM / unit codes) imported into `play/` and Godot.
