# Star

**Entity Type:** Star

## Overview

See also: [[StarSystem]], [[Planet]]

Stars are the primary light sources and gravitational centers of star systems. They are generated based on the "star-ness" value from the multi-dimensional noise map.

## Components

- **Position**
- **Renderable**
- **Star** (contains brightness, spectral type, temperature)
- **Influence Radius** (affects planet formation and resources)

## Generation

```pseudo
if noise.GetValue(x, y, 0) > STAR_THRESHOLD:
    createStarEntity(x, y)
```

See also: [[StarSystem]], [[Planet]], [[Universe_Generation]]