# Structures

**Entity Type:** Structure

## Overview

Structures represent any constructed or artificial entity in the Sleeping Empire universe, including space stations, outposts, ships, research facilities, and ancient ruins. Like all other objects in the game, Structures are **Entities** composed of multiple **Components**.

## Core Components

### Type
Defines the category of the structure:
- Space Station
- Research Facility
- Military Outpost
- Trading Hub
- Ancient Ruin
- Ship (capital or personal)

### Origin
Indicates where the structure came from:
- Imperial (built during the height of the Empire)
- Post-Imperial (constructed after the Emperor's disappearance)
- Foreign (built by another faction)
- Unknown / Ancient

### Purpose
Describes the primary function of the structure:
- Military
- Research
- Trade / Commerce
- Residential
- Religious / Ceremonial
- Storage / Logistics

### Age
Represents how long the structure has existed. This can affect:
- Structural integrity
- Available technology level
- Likelihood of valuable loot or scriptures
- Hostile entities present

## Optional Components

- **Position**
- **Resources** (stored materials or wealth)
- **Defenses** (armaments or shielding)
- **Population**
- **Faction Control**
- **Scriptures** (hidden lore)

## System Interactions

- The **Exploration System** can discover new structures.
- The **Combat System** can interact with structures that have the **Defenses** component.
- The **Loot System** can generate rewards from structures based on their **Age**, **Origin**, and **Resources** components.

See also: [[Player_Character]], [[NPC]], [[Resources]], [[GAME_ARCHITECTURE]]