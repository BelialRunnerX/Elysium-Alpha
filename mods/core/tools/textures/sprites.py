"""Item and block sprites for the Voidforged set."""
import random

from canvas import Canvas, rect, union
from style import METAL, GLOW


# ==========================================================================
# Materials
# ==========================================================================

INGOT = union(
    rect(5, 5, 11, 5),
    rect(4, 6, 12, 6),
    rect(3, 7, 12, 8),
    rect(4, 9, 11, 9),
    rect(5, 10, 10, 10),
)


INGOT_TOP = union(rect(5, 5, 11, 5), rect(4, 6, 12, 6))
INGOT_BODY = INGOT - INGOT_TOP


def ingot(metal, glow):
    """
    A cast bar. The top face is painted as its own plate a full two steps
    lighter than the body — a smooth gradient across the whole silhouette
    reads as a pebble, a hard value break reads as a folded edge.
    """
    ramp = METAL[metal]
    c = Canvas()
    for (x, y) in INGOT_BODY:
        c.put(x, y, ramp[3] if y <= 8 else ramp[2])
    for (x, y) in INGOT_TOP:
        c.put(x, y, ramp[5] if y == 5 else ramp[4])
    # Fold line, then the lit underside of the far edge.
    c.engrave([(x, 7) for x in range(4, 12)], ramp, shade=1)
    c.engrave([(x, 10) for x in range(5, 11)], ramp, shade=1)
    c.glow([(7, 5), (8, 5)], GLOW[glow], within=INGOT_TOP)
    c.outline(ramp[0])
    return c


SHARD = union(
    rect(7, 2, 8, 2),
    rect(6, 3, 9, 3),
    rect(5, 4, 10, 5),
    rect(4, 6, 11, 9),
    rect(5, 10, 10, 11),
    rect(6, 12, 9, 12),
    rect(7, 13, 8, 13),
)


def shard(metal, glow):
    """Voidglass does not cast — it grows. An angular crystal instead of a bar."""
    c = Canvas()
    c.plate(SHARD, METAL[metal])
    # Facet lines running out from the core.
    c.engrave([(6, 5), (7, 4), (8, 4), (9, 5), (6, 10), (9, 10)], METAL[metal], shade=1)
    c.glow([(7, 7), (8, 7), (7, 8), (8, 8)], GLOW[glow], within=SHARD)
    c.outline(METAL[metal][0])
    return c


# ==========================================================================
# Reforge catalyst — a caged core
# ==========================================================================

def _diamond(radius, cx=7.5, cy=7.5):
    return {(x, y) for y in range(16) for x in range(16)
            if abs(x - cx) + abs(y - cy) <= radius}


def catalyst(glow):
    c = Canvas()
    frame = _diamond(6.0) - _diamond(3.0)
    spikes = union(
        rect(7, 0, 8, 1), rect(7, 14, 8, 15),
        rect(0, 7, 1, 8), rect(14, 7, 15, 8),
    )
    housing = frame | spikes
    c.plate(housing, METAL["voidsteel"])
    c.engrave([(7, 2), (8, 2), (7, 13), (8, 13), (2, 7), (2, 8), (13, 7), (13, 8)],
              METAL["voidsteel"], shade=1)
    core = _diamond(2.5)
    c.glow(core, GLOW[glow], within=core | frame)
    c.outline(METAL["voidsteel"][0])
    return c


# ==========================================================================
# Runes — notched obsidian tablets with a carved sigil
# ==========================================================================

TABLET = union(
    rect(3, 2, 12, 13),
) - {(3, 2), (12, 2), (3, 13), (12, 13)}

TABLET_INNER = rect(5, 4, 10, 11)

SIGILS = {
    # Voidward — a warding shield
    "voidward": [(5, 4), (6, 4), (7, 4), (8, 4), (9, 4), (10, 4),
                 (5, 5), (10, 5), (5, 6), (10, 6), (5, 7), (10, 7),
                 (6, 8), (9, 8), (7, 9), (8, 9), (7, 10), (8, 10)],
    # Plasmaforge — a struck bolt
    "plasmaforge": [(9, 4), (8, 5), (7, 6), (8, 6), (9, 6),
                    (6, 7), (7, 7), (8, 7), (7, 8), (6, 9), (7, 9), (6, 10)],
    # Neuralspike — a synapse
    "neuralspike": [(7, 4), (8, 4), (7, 5), (8, 5),
                    (5, 6), (6, 6), (7, 6), (8, 6), (9, 6), (10, 6),
                    (7, 7), (8, 7), (6, 8), (9, 8), (5, 9), (10, 9),
                    (7, 10), (8, 10), (7, 11), (8, 11)],
    # Dimensionalshift — a rift
    "dimensionalshift": [(7, 4), (8, 4), (6, 5), (9, 5), (5, 6), (10, 6),
                         (5, 7), (10, 7), (5, 8), (10, 8), (6, 9), (9, 9),
                         (7, 10), (8, 10)],
    # Kineticsurge — stacked chevrons
    "kineticsurge": [(5, 5), (6, 6), (7, 7), (8, 7), (9, 6), (10, 5),
                     (5, 8), (6, 9), (7, 10), (8, 10), (9, 9), (10, 8)],
}


def rune(sigil, glow):
    c = Canvas()
    c.plate(TABLET, METAL["obsidian"])
    # Carved border, one pixel in from the edge.
    border = set()
    for x in range(4, 12):
        border |= {(x, 3), (x, 12)}
    for y in range(3, 13):
        border |= {(4, y), (11, y)}
    c.engrave(border, METAL["obsidian"], shade=1)
    c.carve(SIGILS[sigil], GLOW[glow], within=TABLET)
    c.outline(METAL["obsidian"][0])
    return c


# ==========================================================================
# Armour icons
# ==========================================================================

HELM = union(
    rect(6, 2, 9, 2),
    rect(5, 3, 10, 3),
    rect(4, 4, 11, 5),
    rect(3, 6, 12, 11),
    rect(4, 12, 11, 12),
    rect(3, 12, 4, 13), rect(11, 12, 12, 13),   # cheek guards
    rect(7, 0, 8, 1),                            # crest spike
)

HELM_VISOR = union(rect(4, 8, 6, 8), rect(9, 8, 11, 8))


def helmet(metal, glow):
    c = Canvas()
    c.plate(HELM, METAL[metal])
    c.engrave([(7, 4), (8, 4), (7, 5), (8, 5), (7, 6), (8, 6)], METAL[metal], shade=1)
    c.engrave([(x, 10) for x in range(4, 12)], METAL[metal], shade=1)
    c.glow(HELM_VISOR, GLOW[glow], within=HELM)
    c.outline(METAL[metal][0])
    return c


CHEST = union(
    rect(5, 3, 10, 12),                          # torso
    rect(1, 3, 4, 7), rect(11, 3, 14, 7),        # pauldrons
    rect(2, 8, 4, 9), rect(11, 8, 13, 9),        # upper arms
    rect(4, 11, 11, 12),                         # skirt flare
    rect(1, 2, 2, 2), rect(13, 2, 14, 2),        # pauldron spikes
) - rect(7, 3, 8, 3)                             # neck opening


def chestplate(metal, glow):
    c = Canvas()
    c.plate(CHEST, METAL[metal])
    c.engrave([(4, y) for y in range(4, 11)] + [(11, y) for y in range(4, 11)],
              METAL[metal], shade=1)
    c.engrave([(x, 10) for x in range(5, 11)], METAL[metal], shade=1)
    c.highlight([(2, 3), (3, 3), (12, 3), (13, 3)], METAL[metal], shade=5)
    c.glow([(7, 6), (8, 6), (7, 7), (8, 7)], GLOW[glow], within=CHEST)
    c.outline(METAL[metal][0])
    return c


LEGS = union(
    rect(3, 2, 12, 4),                           # belt
    rect(3, 5, 6, 13), rect(9, 5, 12, 13),       # legs
    rect(2, 2, 2, 3), rect(13, 2, 13, 3),        # hip plates
)


def leggings(metal, glow):
    c = Canvas()
    c.plate(LEGS, METAL[metal])
    c.engrave([(x, 4) for x in range(3, 13)], METAL[metal], shade=1)
    c.engrave([(3, y) for y in (10, 11)] + [(12, y) for y in (10, 11)], METAL[metal], shade=1)
    c.glow([(3, 7), (3, 8), (12, 7), (12, 8)], GLOW[glow], within=LEGS)
    c.outline(METAL[metal][0])
    return c


BOOTS = union(
    rect(2, 4, 5, 9), rect(10, 4, 13, 9),        # shins
    rect(1, 10, 6, 13), rect(9, 10, 14, 13),     # feet, with a gap between them
    rect(2, 3, 3, 3), rect(12, 3, 13, 3),        # knee lip
)


def boots(metal, glow):
    c = Canvas()
    c.plate(BOOTS, METAL[metal])
    c.engrave([(x, 10) for x in range(1, 7)] + [(x, 10) for x in range(9, 15)],
              METAL[metal], shade=1)
    c.engrave([(x, 13) for x in range(1, 7)] + [(x, 13) for x in range(9, 15)],
              METAL[metal], shade=1)
    c.glow([(2, 7), (2, 8), (13, 7), (13, 8)], GLOW[glow], within=BOOTS)
    c.outline(METAL[metal][0])
    return c


CROWN = union(
    rect(2, 9, 13, 12),                          # band
    rect(6, 4, 9, 8), rect(7, 2, 8, 3),          # centre spire and its tip
    rect(2, 5, 3, 8), rect(12, 5, 13, 8),        # side spires
)


def crown(metal, glow):
    c = Canvas()
    c.plate(CROWN, METAL[metal])
    c.engrave([(x, 12) for x in range(3, 13)], METAL[metal], shade=1)
    c.highlight([(x, 9) for x in range(3, 13)], METAL[metal], shade=5)
    c.glow([(7, 10), (8, 10), (7, 11), (8, 11)], GLOW[glow], within=CROWN)
    c.glow([(7, 2), (8, 2)], GLOW[glow], within=CROWN, halo=False)
    c.outline(METAL[metal][0])
    return c


# ==========================================================================
# Blocks
# ==========================================================================

def _clustered_stone(seed, ramp):
    """
    Per-pixel noise reads as static. One smoothing pass turns it into the
    clustered blotches that vanilla stone actually has.
    """
    rng = random.Random(seed)
    grid = [[rng.randrange(1, 6) for _ in range(16)] for _ in range(16)]
    smoothed = [[0] * 16 for _ in range(16)]
    for y in range(16):
        for x in range(16):
            window = [grid[(y + dy) % 16][(x + dx) % 16]
                      for dy in (-1, 0, 1) for dx in (-1, 0, 1)]
            window.sort()
            smoothed[y][x] = window[4]
    c = Canvas()
    for y in range(16):
        for x in range(16):
            c.put(x, y, ramp[smoothed[y][x]])
    return c


ORE_CLUSTERS = [
    [(2, 3), (3, 3), (2, 4), (3, 4), (4, 4), (3, 5)],
    [(10, 2), (11, 2), (12, 2), (11, 3), (12, 3), (11, 4)],
    [(6, 7), (7, 7), (8, 7), (6, 8), (7, 8), (8, 8), (7, 9)],
    [(1, 10), (2, 10), (1, 11), (2, 11), (3, 11)],
    [(11, 9), (12, 9), (13, 9), (12, 10), (13, 10)],
    [(6, 12), (7, 12), (8, 12), (7, 13), (8, 13)],
]


def ore(glow, seed):
    """Crystal veins bedded into stone, each with a lit core."""
    c = _clustered_stone(seed, METAL["stone"])
    ramp = GLOW[glow]
    everything = rect(0, 0, 15, 15)
    for cluster in ORE_CLUSTERS:
        # A dark rim first, so the crystal sits *in* the rock rather than on it.
        rim = set()
        for (x, y) in cluster:
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    if (x + dx, y + dy) not in cluster:
                        rim.add((x + dx, y + dy))
        for (x, y) in rim:
            c.put(x, y, METAL["stone"][0])
        for (x, y) in cluster:
            c.put(x, y, ramp[2])
        core = cluster[:2]
        c.glow(core, ramp, within=set(cluster), halo=False)
    return c


def storage_block(metal, glow, seed=4):
    """Four riveted plates with an energy seam running between them."""
    ramp = METAL[metal]
    c = Canvas()
    for y in range(16):
        for x in range(16):
            c.put(x, y, ramp[4] if y % 8 < 3 else ramp[3])
    rng = random.Random(seed)
    for y in range(16):
        for x in range(16):
            if rng.random() < 0.12:
                c.put(x, y, ramp[2])

    seam = union(rect(7, 0, 8, 15), rect(0, 7, 15, 8))
    for (x, y) in seam:
        c.put(x, y, METAL[metal][1])
    c.glow([(7, 7), (8, 7), (7, 8), (8, 8)], GLOW[glow], within=seam, halo=True)

    for (x, y) in [(2, 2), (13, 2), (2, 13), (13, 13),
                   (5, 5), (10, 5), (5, 10), (10, 10)]:
        c.put(x, y, METAL[metal][5])
        c.put(x, y + 1, METAL[metal][1])

    for x in range(16):
        c.put(x, 0, METAL[metal][5])
        c.put(x, 15, METAL[metal][0])
    for y in range(16):
        c.put(0, y, METAL[metal][4])
        c.put(15, y, METAL[metal][0])
    return c


WORKSTATION_INLAY = {
    # Reforge Table — an anvil face ringed by containment
    "reforge_table": ([(5, 5), (6, 5), (7, 5), (8, 5), (9, 5), (10, 5),
                       (5, 10), (6, 10), (7, 10), (8, 10), (9, 10), (10, 10),
                       (5, 6), (5, 7), (5, 8), (5, 9),
                       (10, 6), (10, 7), (10, 8), (10, 9)],
                      [(7, 7), (8, 7), (7, 8), (8, 8)]),
    # Rune Socket Table — an open socket ring
    "rune_socket_table": ([(6, 4), (7, 4), (8, 4), (9, 4),
                           (5, 5), (10, 5), (4, 6), (11, 6),
                           (4, 7), (11, 7), (4, 8), (11, 8),
                           (5, 9), (10, 9), (6, 10), (7, 10), (8, 10), (9, 10),
                           (6, 11), (9, 11)],
                          [(7, 7), (8, 7), (7, 8), (8, 8)]),
    # Ascension Forge — an upward chevron over a crucible
    # Ascension Forge — three stacked chevrons. Anything more literal than
    # "up" turned into a bug shape at this size.
    "ascension_forge": ([(7, 2), (8, 2), (6, 3), (9, 3), (5, 4), (10, 4),
                         (7, 6), (8, 6), (6, 7), (9, 7), (5, 8), (10, 8),
                         (7, 10), (8, 10), (6, 11), (9, 11), (5, 12), (10, 12)],
                        [(7, 2), (8, 2)]),
}


def workstation(name, metal, glow, seed=9):
    """
    Note the flat base rather than a shaded plate: a block texture tiles, and a
    vertical gradient across 16 pixels produces a hard band at every seam once
    two of them sit side by side.
    """
    ramp = METAL[metal]
    c = Canvas()
    for y in range(16):
        for x in range(16):
            c.put(x, y, ramp[3])
    rng = random.Random(seed)
    for y in range(16):
        for x in range(16):
            roll = rng.random()
            if roll < 0.16:
                c.put(x, y, ramp[2])
            elif roll < 0.24:
                c.put(x, y, ramp[4])

    inlay, core = WORKSTATION_INLAY[name]
    c.carve(inlay, GLOW[glow], within=rect(0, 0, 15, 15))
    c.glow(core, GLOW[glow], within=rect(0, 0, 15, 15))

    # Bevelled rim so the block reads as a worked surface.
    for x in range(16):
        c.put(x, 0, METAL[metal][5])
        c.put(x, 15, METAL[metal][0])
    for y in range(16):
        c.put(0, y, METAL[metal][4])
        c.put(15, y, METAL[metal][0])
    c.put(0, 0, METAL[metal][5])
    c.put(15, 15, METAL[metal][0])
    return c


# ==========================================================================
# Weapons
#
# Minecraft item sprites read on the bottom-left → top-right diagonal, so the
# blades are built from a 45-degree axis rather than an upright silhouette.
# ==========================================================================

def _axis(x0, y0, length, width=2, dx=1, dy=-1):
    """
    A band of pixels along a 45-degree diagonal, `width` pixels thick.

    Thickness runs along x, not along the diagonal: offsetting diagonally
    leaves each step touching only at its corners, and a corner-connected run
    reads as a dotted line — and the outline pass then floods the gaps.
    """
    points = set()
    for i in range(length):
        bx, by = x0 + dx * i, y0 + dy * i
        for w in range(width):
            points.add((bx + w, by))
    return {(x, y) for (x, y) in points if 0 <= x < 16 and 0 <= y < 16}


SWORD_BLADE = _axis(5, 11, 8, width=3)
SWORD_TIP = {(13, 3), (12, 3), (13, 2)}
SWORD_GUARD = _axis(2, 10, 5, width=2, dx=1, dy=1)
SWORD_GRIP = union(rect(1, 12, 2, 14))
SWORD_POMMEL = union(rect(0, 14, 1, 15))
SWORD = SWORD_BLADE | SWORD_TIP | SWORD_GUARD | SWORD_GRIP | SWORD_POMMEL


def sword(metal, glow):
    """The baseline elemental weapon: a straight blade with a lit fuller."""
    c = Canvas()
    c.band(SWORD_BLADE | SWORD_TIP, METAL[metal])
    c.band(SWORD_GUARD, METAL[metal], light=4, mid=2, dark=1)
    c.plate(SWORD_GRIP | SWORD_POMMEL, METAL[metal])
    # The fuller — the groove down the centre of the blade — carries the charge.
    fuller = _axis(6, 11, 7, width=1)
    c.carve(fuller, GLOW[glow], within=SWORD_BLADE)
    c.engrave(SWORD_GRIP, METAL[metal], shade=1)
    c.highlight(SWORD_POMMEL, METAL[metal], shade=4)
    c.outline(METAL[metal][0])
    return c


LASH_BLADE = _axis(5, 11, 8, width=2) | _axis(6, 10, 7, width=1)
LASH_BARBS = {(7, 9), (9, 7), (11, 5)}
LASH = LASH_BLADE | {(13, 3), (12, 3)} | SWORD_GUARD | SWORD_GRIP | SWORD_POMMEL


def lash(metal, glow):
    """Neural Lash: a segmented blade, thinner and faster than a sword."""
    c = Canvas()
    c.band(LASH_BLADE | {(13, 3), (12, 3)}, METAL[metal])
    c.band(SWORD_GUARD, METAL[metal], light=4, mid=2, dark=1)
    c.plate(SWORD_GRIP | SWORD_POMMEL, METAL[metal])
    c.glow(sorted(LASH_BARBS), GLOW[glow], within=LASH)
    c.outline(METAL[metal][0])
    return c


MAUL_HAFT = _axis(1, 14, 10, width=2)
MAUL_HEAD = union(rect(9, 2, 14, 6), rect(10, 1, 13, 1))
MAUL = MAUL_HAFT | MAUL_HEAD | union(rect(0, 14, 1, 15))


def maul(metal, glow):
    """Kinetic Maul: most of the mass at the far end of the swing."""
    c = Canvas()
    c.band(MAUL_HAFT | union(rect(0, 14, 1, 15)), METAL[metal])
    c.plate(MAUL_HEAD, METAL[metal])
    # Striking face on the far side, bound by two bands.
    c.highlight([(14, y) for y in range(2, 7)], METAL[metal], shade=5)
    c.engrave([(10, y) for y in range(1, 7)] + [(13, y) for y in range(1, 7)],
              METAL[metal], shade=1)
    c.glow([(11, 3), (12, 3), (11, 4), (12, 4)], GLOW[glow], within=MAUL_HEAD)
    c.outline(METAL[metal][0])
    return c


LANCE_SHAFT = _axis(1, 13, 10, width=2)
LANCE_HEAD = union(rect(11, 2, 13, 4), rect(12, 1, 14, 3), rect(13, 0, 14, 1))
LANCE_COLLAR = union(rect(9, 5, 11, 6))
LANCE = LANCE_SHAFT | LANCE_HEAD | LANCE_COLLAR | union(rect(0, 14, 1, 15))


def lance(metal, glow):
    """
    Singularity Lance. The archive gives it one attack per turn and the
    highest damage on the board, so the sprite is nearly all reach with the
    weight concentrated in the head.
    """
    c = Canvas()
    c.band(LANCE_SHAFT | union(rect(0, 14, 1, 15)), METAL[metal])
    c.plate(LANCE_HEAD, METAL[metal])
    c.plate(LANCE_COLLAR, METAL[metal])
    c.engrave(sorted(LANCE_COLLAR), METAL[metal], shade=1)
    c.glow([(12, 2), (13, 1)], GLOW[glow], within=LANCE_HEAD)
    c.glow([(5, 9), (6, 8)], GLOW[glow], within=LANCE_SHAFT, halo=False)
    c.outline(METAL[metal][0])
    return c


RIFLE_BODY = _axis(4, 11, 6, width=3)
RIFLE_BARREL = _axis(10, 5, 5, width=2)
RIFLE_STOCK = union(rect(1, 12, 3, 14), rect(0, 14, 1, 15))
RIFLE_GRIP = union(rect(4, 13, 5, 15))
RIFLE_SIGHT = {(9, 5), (10, 4)}
RIFLE = RIFLE_BODY | RIFLE_BARREL | RIFLE_STOCK | RIFLE_GRIP | RIFLE_SIGHT


def rifle(metal, glow):
    """
    Neural Cascade Rifle. Two attacks per turn in the archive, so it reads
    light: a slim receiver, a long thin barrel, an emitter at the muzzle.
    """
    c = Canvas()
    c.band(RIFLE_BODY, METAL[metal])
    c.band(RIFLE_BARREL, METAL[metal], light=4, mid=2, dark=1)
    c.plate(RIFLE_STOCK | RIFLE_GRIP, METAL[metal])
    for (x, y) in RIFLE_SIGHT:
        c.put(x, y, METAL[metal][4])
    c.engrave(sorted(RIFLE_GRIP), METAL[metal], shade=1)
    # Charge cells along the receiver, emitter at the muzzle.
    c.glow([(6, 10), (7, 9)], GLOW[glow], within=RIFLE_BODY)
    c.glow([(13, 2), (14, 1)], GLOW[glow], within=RIFLE_BARREL)
    c.outline(METAL[metal][0])
    return c


# ==========================================================================
# Utility rune sigils — the archive's non-elemental runes
# ==========================================================================

SIGILS.update({
    # Stabilizer — a closed cross, the Imperial medical mark
    "stabilizer": [(7, 5), (8, 5), (7, 6), (8, 6),
                   (5, 7), (6, 7), (7, 7), (8, 7), (9, 7), (10, 7),
                   (5, 8), (6, 8), (7, 8), (8, 8), (9, 8), (10, 8),
                   (7, 9), (8, 9), (7, 10), (8, 10)],
    # Reflex — two offset chevrons, a step sideways
    "reflex": [(6, 4), (5, 5), (6, 6), (7, 7), (6, 8), (5, 9), (6, 10),
               (10, 5), (9, 6), (10, 7), (9, 8), (10, 9)],
    # Barrier — a shield boss
    "barrier": [(5, 4), (6, 4), (7, 4), (8, 4), (9, 4), (10, 4),
                (5, 5), (10, 5), (5, 6), (10, 6), (5, 7), (10, 7),
                (6, 8), (9, 8), (7, 9), (8, 9), (7, 10), (8, 10),
                (7, 6), (8, 6), (7, 7), (8, 7)],
    # Plasma Core — a contained sun
    "plasma_core": [(7, 4), (8, 4), (5, 5), (10, 5), (4, 7), (11, 7),
                    (5, 9), (10, 9), (7, 10), (8, 10),
                    (7, 6), (8, 6), (6, 7), (9, 7), (7, 8), (8, 8),
                    (6, 6), (9, 6), (6, 8), (9, 8)],
})


# ==========================================================================
# Voidweave Aegis — the archive's flagship chestplate
# ==========================================================================

AEGIS = union(
    CHEST,
    rect(3, 10, 12, 12),                         # layered fauld across the waist
    rect(5, 2, 6, 2), rect(9, 2, 10, 2),         # raised collar
)


def aegis(metal, glow):
    """
    Heavier than the standard chestplate: layered plate, a wider core, and a
    second glow at the collar so it reads as the higher-tier piece at a
    glance.
    """
    c = Canvas()
    c.plate(AEGIS, METAL[metal])
    c.engrave([(4, y) for y in range(4, 12)] + [(11, y) for y in range(4, 12)],
              METAL[metal], shade=1)
    c.engrave([(x, 9) for x in range(5, 11)], METAL[metal], shade=1)
    c.highlight([(2, 3), (3, 3), (12, 3), (13, 3)], METAL[metal], shade=5)
    c.glow([(7, 6), (8, 6), (7, 7), (8, 7), (7, 8), (8, 8)], GLOW[glow], within=AEGIS)
    c.glow([(6, 4), (9, 4)], GLOW[glow], within=AEGIS, halo=False)
    c.outline(METAL[metal][0])
    return c


# ==========================================================================
# Area tools
#
# Four shapes, three materials each. They share one haft so the line reads as
# a set, and are told apart entirely by the head — which is the part a player
# actually sees at inventory size.
# ==========================================================================

TOOL_HAFT = _axis(1, 14, 10, width=2)
TOOL_POMMEL = union(rect(0, 14, 1, 15))
TOOL_GRIP = TOOL_HAFT | TOOL_POMMEL


HAMMER_HEAD = union(
    rect(8, 3, 15, 7),
    rect(9, 2, 14, 2),          # crown of the head
    rect(9, 8, 14, 8),          # and its underside
)
HAMMER = TOOL_GRIP | HAMMER_HEAD


def hammer(metal, glow):
    """
    A brick of a head with a striking face at each end. All the weight is out
    past the haft, which is what a 3x3 swing should look like.
    """
    c = Canvas()
    c.band(TOOL_GRIP, METAL[metal])
    c.plate(HAMMER_HEAD, METAL[metal])
    # Both faces bright, the bindings that hold them dark: reads as forged
    # rather than cast.
    c.highlight([(15, y) for y in range(3, 8)], METAL[metal], shade=5)
    c.highlight([(8, y) for y in range(3, 8)], METAL[metal], shade=4)
    c.engrave([(10, y) for y in range(2, 9)] + [(13, y) for y in range(2, 9)],
              METAL[metal], shade=1)
    c.glow([(11, 5), (12, 5)], GLOW[glow], within=HAMMER_HEAD)
    c.outline(METAL[metal][0])
    return c


# The outline pass grows every silhouette by a pixel in each direction, so
# these shapes are drawn a pixel lean and gaps are never left at 1px — see the
# art rules in TEXTURES.md.

BROADAXE_BIT = union(
    rect(3, 0, 9, 0),           # flat back of the bit
    rect(3, 1, 10, 1),
    rect(4, 2, 10, 2),
    rect(5, 3, 10, 3),          # underside runs parallel to the haft
)
BROADAXE_BEARD = union(rect(11, 1, 12, 2))
BROADAXE = TOOL_GRIP | BROADAXE_BIT | BROADAXE_BEARD


def broadaxe(metal, glow):
    """
    A wedge sitting on top of the haft: flat along the back, edge along the
    left, underside sloping into the handle. Asymmetry is what stops it
    reading as a leaf.
    """
    c = Canvas()
    c.band(TOOL_GRIP, METAL[metal])
    c.plate(BROADAXE_BIT | BROADAXE_BEARD, METAL[metal])
    c.highlight([(3, 0), (3, 1), (4, 0), (4, 1), (4, 2), (5, 3)],
                METAL[metal], shade=5)
    c.engrave([(9, y) for y in range(1, 4)], METAL[metal], shade=1)
    c.glow([(7, 1), (8, 1)], GLOW[glow], within=BROADAXE_BIT)
    c.outline(METAL[metal][0])
    return c


SCYTHE_SNATH = _axis(1, 14, 10, width=2)
# A single-pixel arc: the outline pass thickens it to three, which is exactly
# the weight a scythe blade wants. Each run overlaps the next in x so the
# curve stays orthogonally connected and the outline cannot flood it.
SCYTHE_BLADE = union(
    rect(9, 4, 11, 4),          # rooted at the snath
    rect(4, 3, 9, 3),           # sweeping left
    rect(2, 2, 4, 2),           # rising
    rect(1, 3, 2, 3),           # and hooking down to the point
)
SCYTHE = SCYTHE_SNATH | TOOL_POMMEL | SCYTHE_BLADE


def scythe(metal, glow):
    """A long thin arc sweeping off the snath, hooked at the point."""
    c = Canvas()
    c.band(SCYTHE_SNATH | TOOL_POMMEL, METAL[metal])
    c.plate(SCYTHE_BLADE, METAL[metal])
    c.highlight([(2, 2), (3, 2), (4, 2), (1, 3)], METAL[metal], shade=5)
    c.glow([(10, 4)], GLOW[glow], within=SCYTHE_BLADE, halo=False)
    c.outline(METAL[metal][0])
    return c


# A one-pixel shaft, so the head reads as a bulge rather than more shaft.
SPEAR_SHAFT = _axis(1, 14, 11, width=1)
SPEAR_HEAD = union(
    rect(11, 4, 12, 4),
    rect(10, 3, 13, 3),
    rect(11, 2, 14, 2),
    rect(12, 1, 15, 1),
    rect(14, 0, 15, 0),
)
SPEAR = SPEAR_SHAFT | TOOL_POMMEL | SPEAR_HEAD


def spear(metal, glow):
    """
    A broad leaf head on a thin shaft. The head is nearly three times the
    shaft's width, which is the only thing that separates a spear from a stick
    at this size.
    """
    c = Canvas()
    c.band(SPEAR_SHAFT | TOOL_POMMEL, METAL[metal])
    c.plate(SPEAR_HEAD, METAL[metal])
    c.highlight([(15, 0), (14, 0), (15, 1), (14, 1)], METAL[metal], shade=5)
    c.glow([(12, 2), (11, 3)], GLOW[glow], within=SPEAR_HEAD, halo=False)
    c.outline(METAL[metal][0])
    return c


# The material each tool variant is drawn in: metal ramp, then glow ramp.
TOOL_MATERIALS = {
    "voidglass": ("obsidian", "void"),
    "aetherium": ("aetherium", "dimensional"),
    "neutronium": ("neutronium", "kinetic"),
}

TOOL_SHAPES = {
    "hammer": hammer,
    "broadaxe": broadaxe,
    "scythe": scythe,
    "spear": spear,
}


# ==========================================================================
# The Imperial Codex
# ==========================================================================

CODEX_COVER = union(
    rect(3, 2, 12, 13),
)
CODEX_SPINE = union(rect(3, 2, 4, 13))
CODEX_PAGES = union(rect(11, 3, 12, 12))
# The Code's own mark: a serif I, which is what the Empire stamps on anything
# it considers filed. A cross read as a medical symbol, which is the one thing
# this book is not.
CODEX_SIGIL = union(
    rect(6, 5, 9, 5),
    rect(7, 6, 8, 9),
    rect(6, 10, 9, 10),
)


def codex(metal, glow):
    """
    A slab of a book: heavy board, a lit spine, and the Code's mark cut into
    the cover rather than printed on it. Reads as a document at inventory size
    because the page block on the fore-edge is the only bright thing on it.
    """
    c = Canvas()
    c.plate(CODEX_COVER, METAL[metal])
    c.engrave(sorted(CODEX_SPINE), METAL[metal], shade=1)
    c.highlight(sorted(CODEX_PAGES), METAL[metal], shade=5)
    c.carve(CODEX_SIGIL, GLOW[glow], within=CODEX_COVER)
    c.glow([(4, 7), (4, 8)], GLOW[glow], within=CODEX_SPINE, halo=False)
    c.outline(METAL[metal][0])
    return c
