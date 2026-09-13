# Elysium – Core Architecture Pseudo Code (Explained)

## 1. High-Level Architecture

**Core Idea**: Elysium is a triple A voxel-based game using an **Entity Component System (ECS)**. Everything — from the player to planets and even individual voxels — is represented through entities and components.

```pseudo
Universe
├── Seed
├── Noise Generator (Multi-dimensional)
├── Entities (Player, NPCs, Planets, Stars, Voxels, Structures...)
└── Systems (Movement, Combat, Loot, Voxel Rendering, etc.)
```

## 2. Component Attachment Rules

- **Irrelevant components** are **not attached**.
- **Optional components** may be attached but can be set to `null` when not in use.

## 3. Core Components

### Position Component
```pseudo
struct Position {
    float x, y, z
}
```

### Voxel Component
```pseudo
struct Voxel {
    int type
    float density
}
```

### Planet Component
```pseudo
struct Planet {
    float planetNess
    array<float, 20> mineralPercentages
}
```

### Armor Component
```pseudo
struct Armor {
    string name
    int tier
    float armorFlat, armorPercent
    float healthFlat, healthPercent
    float movementSpeedPercent
    int element
    array<Rune> runes
}
```

### Weapon Component
```pseudo
struct Weapon {
    string name
    int tier
    float damageFlat, damagePercent
    float fireRate
    int element
}
```

### Psionic Component
```pseudo
struct Psionic {
    int element
    float power
    float cooldown
}
```

## 4. Universe Generation

```pseudo
function generateUniverse(seed):
    noise = FastNoise(seed)
    
    for x, y in universeMap:
        starValue = noise.GetValue(x, y, 0)
        planetValue = noise.GetValue(x, y, 1)
        
        if starValue > STAR_THRESHOLD:
            createStar(x, y, starValue)
        
        if planetValue > PLANET_THRESHOLD:
            createPlanet(x, y, planetValue)
```

## 5. Voxel Planet Rendering

```pseudo
function generatePlanetVoxels(planet):
    for x, y, z in planetVolume:
        density = noise.GetValue(x, y, z, planet.seed)
        if density > THRESHOLD:
            createVoxelEntity(x, y, z, density)
```

## 6. Combat System

```pseudo
function resolveAttack(attacker, target):
    if attacker has Weapon and target has Armor:
        damage = calculateDamage(attacker.get<Weapon>(), target.get<Armor>())
        target.get<Health>().current -= damage
```

See also: [[GAME_ARCHITECTURE]], [[Player_Character]], [[Armor]], [[Weapon]], [[Psionic]]