# Combat Rules

**System:** Combat System

## Overview

Combat in Elysium is handled by the **Combat System**, which operates on entities that have the required components (typically a [[Weapon]] on the attacker and [[Armor]] on the target).

See also: [[Armor]], [[Weapon]], [[Psionic]], [[GAME_ARCHITECTURE]]

## Core Logic

```pseudo
function resolveAttack(attacker, target):
    if attacker has Weapon and target has Armor:
        // Calculate damage with elemental modifiers
        // Apply armor reduction
        // Apply critical hits if applicable
```

## Elemental Interactions

- Weapons gain bonus damage against mismatched armor elements.
- Armor gains resistance against its own element.
- Critical hits can trigger element-specific effects.