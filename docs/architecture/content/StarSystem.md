# Star System

**Entity Type:** Star System

## Overview

See also: [[Star]], [[Planet]], [[3D_Planet_Viewer]]

A Star System is a high-level container entity that groups together stars, planets, and other celestial bodies. It is generated procedurally from the universe seed using multi-dimensional noise.

## Components

- **Position** (galactic coordinates)
- **Star** (if a star is present)
- **Planets** (list of planet entities)
- **Asteroid Belts** (optional)
- **Resources** (system-wide resource pools)
- **Faction Control** (optional)

## Generation Logic

```pseudo
function generateStarSystem(x, y, seed):
    starValue = noise.GetValue(x, y, 0)
    if starValue > STAR_THRESHOLD:
        createStarEntity(x, y, starValue)
    
    planetValue = noise.GetValue(x, y, 1)
    if planetValue > PLANET_THRESHOLD:
        createPlanetEntity(x, y, planetValue)
```

## Interaction with 3D Viewer

Clicking on a planet within a star system can trigger a transition into a 3D planetary environment.

See also: [[Planet]], [[Star]], [[Universe_Generation]], [[3D_Planet_Viewer]]