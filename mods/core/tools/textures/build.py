#!/usr/bin/env python3
"""Emit the full Voidforged texture set into the mod's resource tree."""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from PIL import Image, ImageDraw

import sprites
import layers
from style import METAL, GLOW

ROOT = "/tmp/elysium_work/work/src/main/resources/assets/elysium/textures"


def out(rel):
    return os.path.join(ROOT, rel)


def main():
    # -- materials ---------------------------------------------------------
    sprites.ingot("neutronium", "inert").save(out("item/neutronium_ingot.png"))
    sprites.ingot("voidsteel", "aetherium").save(out("item/aetherium_ingot.png"))
    sprites.shard("obsidian", "voidglass").save(out("item/voidglass_ingot.png"))

    # -- the character sheet, in the hand ----------------------------------
    sprites.codex("voidsteel", "aetherium").save(out("item/imperial_codex.png"))

    # -- reforge catalyst --------------------------------------------------
    sprites.catalyst("aetherium").save(out("item/elysium_reforge.png"))

    # -- runes -------------------------------------------------------------
    for sigil, glow in [("voidward", "void"),
                        ("plasmaforge", "plasma"),
                        ("neuralspike", "neural"),
                        ("dimensionalshift", "dimensional"),
                        ("kineticsurge", "kinetic"),
                        # Utility runes from the equipment archive. They get
                        # aetherium rather than an element colour, so a glance
                        # at the inventory separates the two families.
                        ("stabilizer", "aetherium"),
                        ("reflex", "aetherium"),
                        ("barrier", "aetherium"),
                        ("plasma_core", "plasma")]:
        sprites.rune(sigil, glow).save(out(f"item/{sigil}_rune.png"))

    # -- weapons -----------------------------------------------------------
    sprites.sword("voidsteel", "void").save(out("item/voidcut_blade.png"))
    sprites.sword("voidsteel", "plasma").save(out("item/plasma_brand.png"))
    sprites.lash("voidsteel", "neural").save(out("item/neural_lash.png"))
    sprites.sword("voidsteel", "dimensional").save(out("item/rift_edge.png"))
    sprites.maul("neutronium", "kinetic").save(out("item/kinetic_maul.png"))
    sprites.lance("neutronium", "dimensional").save(out("item/singularity_lance.png"))
    sprites.rifle("neutronium", "neural").save(out("item/neural_cascade_rifle.png"))

    # -- area tools: four shapes x three materials -------------------------
    for material, (metal, glow) in sprites.TOOL_MATERIALS.items():
        for shape, draw_tool in sprites.TOOL_SHAPES.items():
            draw_tool(metal, glow).save(out(f"item/{material}_{shape}.png"))

    # -- Elysium armour: one element per piece -----------------------------
    sprites.helmet("voidsteel", "void").save(out("item/elysium_helmet.png"))
    sprites.chestplate("voidsteel", "plasma").save(out("item/plasma_chestplate.png"))
    sprites.leggings("voidsteel", "neural").save(out("item/neural_leggings.png"))
    sprites.boots("voidsteel", "dimensional").save(out("item/dimensional_boots.png"))
    sprites.crown("voidsteel", "void").save(out("item/emperor_crown.png"))
    sprites.aegis("voidsteel", "void").save(out("item/voidweave_aegis.png"))

    # -- Neutronium armour: inert, no elemental colour ---------------------
    sprites.helmet("neutronium", "inert").save(out("item/neutronium_helmet.png"))
    sprites.chestplate("neutronium", "inert").save(out("item/neutronium_chestplate.png"))
    sprites.leggings("neutronium", "inert").save(out("item/neutronium_leggings.png"))
    sprites.boots("neutronium", "inert").save(out("item/neutronium_boots.png"))

    # -- blocks ------------------------------------------------------------
    sprites.ore("inert", 3).save(out("block/neutronium_ore.png"))
    sprites.ore("aetherium", 17).save(out("block/aetherium_ore.png"))
    sprites.ore("voidglass", 29).save(out("block/voidglass_ore.png"))
    sprites.storage_block("neutronium", "inert").save(out("block/neutronium_block.png"))
    sprites.workstation("reforge_table", "voidsteel", "aetherium").save(
        out("block/reforge_table.png"))
    sprites.workstation("rune_socket_table", "obsidian", "voidglass").save(
        out("block/rune_socket_table.png"))
    sprites.workstation("ascension_forge", "voidsteel", "plasma").save(
        out("block/ascension_forge.png"))

    # -- armour layers -----------------------------------------------------
    layers.layer_one("voidsteel", "void").save(out("models/armor/elysium_layer_1.png"))
    layers.layer_two("voidsteel", "void").save(out("models/armor/elysium_layer_2.png"))
    layers.layer_one("neutronium", "inert").save(out("models/armor/neutronium_layer_1.png"))
    layers.layer_two("neutronium", "inert").save(out("models/armor/neutronium_layer_2.png"))

    # -- GUI ---------------------------------------------------------------
    gui_panel().save(out("gui/reforge_table.png"))

    print("voidforged texture set written")


def gui_panel():
    """
    A container panel in the mod's own palette. Slot geometry stays exactly
    vanilla — players read slots by shape, and moving them helps nobody.
    """
    img = Image.new("RGBA", (256, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)

    def c(value):
        value = value.lstrip("#")
        return tuple(int(value[i:i + 2], 16) for i in (0, 2, 4)) + (255,)

    panel = c(METAL["voidsteel"][2])
    edge_light = c(METAL["voidsteel"][4])
    edge_dark = c(METAL["voidsteel"][0])
    slot_bg = c(METAL["voidsteel"][1])
    slot_dark = c(METAL["voidsteel"][0])
    slot_light = c(METAL["voidsteel"][4])
    accent = c(GLOW["aetherium"][1])

    w, h = 176, 166
    draw.rectangle([0, 0, w - 1, h - 1], fill=panel)
    draw.line([(0, 0), (w - 1, 0)], fill=edge_light)
    draw.line([(0, 0), (0, h - 1)], fill=edge_light)
    draw.line([(0, h - 1), (w - 1, h - 1)], fill=edge_dark)
    draw.line([(w - 1, 0), (w - 1, h - 1)], fill=edge_dark)

    # Title rule, so the label sits on something.
    draw.line([(7, 16), (w - 8, 16)], fill=accent)

    # Divider above the player inventory.
    draw.line([(7, 78), (w - 8, 78)], fill=edge_dark)
    draw.line([(7, 79), (w - 8, 79)], fill=edge_light)

    def slot(x, y, lit=False):
        draw.rectangle([x, y, x + 17, y + 17], fill=slot_bg)
        draw.line([(x, y), (x + 17, y)], fill=slot_dark)
        draw.line([(x, y), (x, y + 17)], fill=slot_dark)
        draw.line([(x, y + 17), (x + 17, y + 17)], fill=slot_light)
        draw.line([(x + 17, y), (x + 17, y + 17)], fill=slot_light)
        if lit:
            draw.rectangle([x + 1, y + 1, x + 16, y + 16], outline=accent)

    # Working slots — coordinates mirror ReforgeTableMenu, offset by one.
    for sx in (29, 79, 129):
        slot(sx, 34, lit=True)

    for row in range(3):
        for col in range(9):
            slot(7 + col * 18, 83 + row * 18)
    for col in range(9):
        slot(7 + col * 18, 141)

    return img


if __name__ == "__main__":
    main()
