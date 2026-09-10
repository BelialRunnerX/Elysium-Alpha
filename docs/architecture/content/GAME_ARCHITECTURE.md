# Elysium – Game Architecture

## Overview

Elysium is a **triple A voxel-based video game** built with an **Entity Component System (ECS)** architecture. It features a single massive procedural universe with deep systems for armor, weapons, and psionics.

The game draws inspiration from modded Minecraft but aims for **UE5-level voxel depth** and **disgustingly good optimization**.

## Core Philosophy

- Everything is an **Entity** with **Components**.
- Systems operate only on entities that have the required components.
- Irrelevant components are not attached.
- Optional components can be null when not in use.

See also: [[ARCHITECTURE_PSEUDOCODE]], [[ECS_Overview]]

## World Structure

### Universe
- Single massive procedural universe
- Generated from a single seed using multi-dimensional noise
- Contains star systems, planets, and resources

### Star Systems
- Procedurally generated
- Contain stars, planets, and asteroid fields
- Visible on a 2D space map when "star-ness" exceeds threshold

### Planets
- Voxel-based terrain with high detail
- Generated based on "planet-ness" and mineral data
- Support for 3D exploration and interaction

### Resources & Minerals
- 20+ sci-fi minerals with percentage distribution per planet
- Broader resource system including food, fuel, credits, and influence

See also: [[StarSystem]], [[Planet]], [[Resources]], [[Minerals]], [[Universe_Generation]]

## Core Systems

### Character Stats
- Strength, Reflexes, Intelligence, Willpower, Presence

### Armor System
- Tiers, elemental affinities, rune slots, flat + percentage stats

### Weapons System
- Tiers, elemental damage, fire rate, effectiveness vs armor

### Psionics System
- Elemental abilities with synergies and counters

See also: [[Armor]], [[Weapon]], [[Psionic]], [[Combat_System]]

## Technical Foundation

- **Language**: C++
- **Architecture**: Entity Component System (EnTT or similar)
- **Rendering**: High-fidelity voxel rendering with UE5-level quality and optimization
- **World Generation**: Multi-dimensional noise for stars, planets, and resources

## Current Focus Areas

- Procedural universe generation
- Voxel planet rendering and interaction
- Deep gear and psionics systems
- Efficient ECS implementation for large-scale worlds

See also: [[ARCHITECTURE_PSEUDOCODE]]