# Player Character

**Entity Type:** Player Character  
**Core Drive:** Pursuit of power and influence in the Sleeping Empire

## Overview

See also: [[Resources]], [[Minerals]]

The player character is an ambitious wanderer with no fixed allegiance. Their defining trait is the drive to grow stronger through wealth, technology, psionics, or political connections.

## Components

The Player Character entity is composed of the following components:

### Core Components
- **Position**
- **Health**
- **Power** (Psionic / Energy resource)
- **Stats** (Strength, Reflexes, Intelligence, Willpower, Presence)

### Equipment Components
- **Armor**
- **Weapon**
- **Psionic**

### Progression Components
- **Inventory**
- **Resources** (Credits, Food, Influence, Minerals)
- **Scriptures** (collected lore fragments)

### Optional Components
- **Status Effects**
- **Faction Reputation**
- **Active Quests**

## Component Relationships

- `Health` and `Power` are directly modified by **Armor** and **Psionic** components.
- `Stats` influence the outcome of encounters and the effectiveness of gear.
- `Resources` are consumed and gained through exploration, trade, and combat.
- `Scriptures` provide lore and occasional mechanical benefits.

See also: [[NPC]], [[Resources]], [[Armor]], [[Weapon]], [[Psionic]], [[Combat_System]]