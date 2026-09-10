# Coffee Mug

**Entity Type:** Object / Item

## Overview

A simple everyday object that demonstrates how even mundane items can be represented in the ECS architecture.

## Components

- **Position**
- **Renderable**
- **Name**
- **Contents** (optional — can be null if empty)
- **Temperature** (optional)
- **Owner** (optional — references another entity)

## System Interactions

- The **Rendering System** draws the mug if it has a `Renderable` component.
- The **Physics System** can affect the mug if it has a `Position` and is not held.
- The **Inventory System** can store the mug if the player interacts with it.

See also: [[Player_Character]], [[Resources]]