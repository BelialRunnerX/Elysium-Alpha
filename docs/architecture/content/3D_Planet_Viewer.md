# 3D Planet Viewer

**Feature:** Interactive 3D Environment

## Overview

See also: [[StarSystem]], [[Planet]]

Clicking on a planet in the 2D space map transitions the player into a 3D view of that planet's surface or orbit. This allows for deeper exploration, resource scanning, and potential planetary missions.

## Components

- **Planet** (reference to the planet entity)
- **Camera** (3D viewpoint)
- **Terrain** (generated mesh or voxel data)
- **Resources** (visible resource nodes on the surface)
- **Entities** (NPCs, structures, or hazards on the planet)

## Transition Logic

```pseudo
function onPlanetClick(planetEntity):
    if planetEntity has Planet component:
        load3DScene(planetEntity)
        transitionTo3DMode()
```

## Features

- Procedural terrain generation based on planet-ness and mineral data
- Resource node visualization
- Possible landing zones and points of interest
- Return to 2D map via exit point

See also: [[Planet]], [[StarSystem]], [[Resources]]