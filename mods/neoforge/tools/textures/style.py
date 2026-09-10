"""
Voidforged — the Elysium art system.

Direction: dark gothic plate, sci-fi emissive cores.

Rules every texture follows:
  * One near-black outline traced around every silhouette. Heavy, unbroken.
  * Metals are desaturated and cool. Five shade steps, lit from the top-left.
  * Colour never carries a material; it only ever carries *energy*. Anything
    saturated on a sprite is glowing.
  * Emissive accents are drawn core-out: a near-white core, a saturated ring,
    then a dim halo bleeding into the metal. Small and sparse — one focal glow
    per sprite, at most two.
  * Silhouettes are notched and angular. Gothic means asymmetry at the edges:
    spikes, crenellations, a broken line rather than a smooth arc.
"""

# --------------------------------------------------------------------------
# Metal ramps: index 0 is the outline, 1 darkest .. 5 brightest highlight
# --------------------------------------------------------------------------
METAL = {
    # Elysium's base alloy — black iron with a violet bloom in the highlights
    "voidsteel": ["#06050b", "#181430", "#2b2450", "#413876", "#5b4f9c", "#7f70c4"],
    # Neutronium — denser, colder, almost no hue
    "neutronium": ["#040507", "#14171e", "#232831", "#353c49", "#4e5766", "#727d8e"],
    # Carved obsidian, used for rune tablets and workstation stone
    "obsidian": ["#040309", "#0f0c19", "#191428", "#241d38", "#31284b", "#43376a"],
    # Aetherium — pale planar alloy, a cold teal cast in the highlights
    "aetherium": ["#03090c", "#0d1f26", "#173440", "#22505f", "#317183", "#4f9cae"],
    # Ordinary stone, for the ore blocks' matrix
    "stone": ["#2c2c30", "#4a4a50", "#5e5e66", "#70707a", "#82828d", "#95959f"],
}

# --------------------------------------------------------------------------
# Glow ramps: index 0 dimmest halo .. 4 white-hot core
# --------------------------------------------------------------------------
GLOW = {
    "void":        ["#25104a", "#4b1f92", "#7a37cc", "#a86ef0", "#e3d2ff"],
    "plasma":      ["#3a0d09", "#8a2712", "#d4521a", "#f28f3e", "#ffdaa6"],
    "neural":      ["#04281d", "#0a6242", "#159c68", "#33d296", "#c6ffe4"],
    "dimensional": ["#06203a", "#0c4a80", "#187dc2", "#3aa6e8", "#cdefff"],
    "kinetic":     ["#372504", "#83570b", "#c28815", "#e6b23c", "#fff0ba"],
    "aetherium":   ["#052a36", "#0b5f75", "#149bb8", "#3ecde6", "#d2f6ff"],
    "voidglass":   ["#180630", "#3a1370", "#6425ae", "#9459e6", "#dfc2ff"],
    # Neutronium is inert. It gets a cold pale rim, never a colour.
    "inert":       ["#1b1e25", "#3a4250", "#5a6472", "#8993a4", "#c6d0dd"],
}

# Which glow belongs to which element id used in Elysium.java
ELEMENT_GLOW = ["void", "plasma", "neural", "dimensional", "kinetic"]
