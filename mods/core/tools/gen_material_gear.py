#!/usr/bin/env python3
"""
Generate everything the material gear needs: sprites, models, lang and recipes.

Run it from anywhere:  python3 tools/gen_material_gear.py

It only ever *adds* files, never overwrites one that exists, so a hand-drawn
sprite always wins over a generated one and re-running it after adding a
material is safe. The material table is read out of
ElysiumMaterials.java rather than duplicated here, so there is exactly one
place a material is declared and this cannot drift from it — a material added
to the Java table and not to a table here would otherwise be an item with no
model, which is a purple-and-black cube in the player's hand.

Recipes are written against the material's **ingredient tag**, never an item.
That is what makes a modded material work: c:ingots/tin resolves when a tin mod
is installed and simply does not resolve when one is not, which is ordinary
vanilla behaviour and needs no special handling. It is also why a recipe here
can reference an item that does not exist in this pack.
"""
import json
import os
import re
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "art"))

import materials as material_art
import sprites
import layers
import style

# Shipped inside the repo it generates for, so the root is one level up.
CORE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(CORE, "src/main/resources")
JAVA = os.path.join(CORE, "src/main/java/com/elysium/core/item/ElysiumMaterials.java")

TEXTURES = os.path.join(RES, "assets/elysium/textures")
MODELS = os.path.join(RES, "assets/elysium/models/item")
LANG = os.path.join(RES, "assets/elysium/lang/en_us.json")
RECIPES = os.path.join(RES, "data/elysium/recipe")

SHAPES = ["hammer", "broadaxe", "scythe", "spear"]
ARMOUR = ["helmet", "chestplate", "leggings", "boots"]

# The element each Elysium element id maps to in the art system's glow ramps.
ELEMENT_GLOW = {
    "VOID": "void",
    "PLASMA": "plasma",
    "NEURAL": "neural",
    "DIMENSIONAL": "dimensional",
    "KINETIC": "kinetic",
    "NONE": "inert",
}


# ---------------------------------------------------------------------------
# Read the material table out of the Java, so there is one source of truth
# ---------------------------------------------------------------------------

def read_materials():
    """
    [(name, element_glow, has_armour, ingredient_tag)] for every generated
    material, in declaration order.

    The three Empire materials are read the same way as the rest; they differ
    only in already having hand-drawn sprites, which the writer below respects.
    """
    source = open(JAVA, encoding="utf-8").read()
    found = []

    # Vanilla and Empire materials: a builder chain or a vanillaMaterial call.
    for match in re.finditer(
            r'ElysiumGearMaterial\.builder\(id\("(\w+)"\)\)(.*?)\.register\(\)',
            source, re.S):
        name, body = match.group(1), match.group(2)
        element = re.search(r"\.element\(ElysiumElements\.(\w+)\)", body)
        ingredient = re.search(r'\.ingredient\(common\("([\w/]+)"\)\)', body)
        found.append((
            name,
            ELEMENT_GLOW.get(element.group(1) if element else "NONE", "inert"),
            ".armour(" in body,
            "c:" + ingredient.group(1) if ingredient else "c:ingots/" + name))

    for match in re.finditer(
            r'vanillaMaterial\(\s*"(\w+)",\s*(?:common\("([\w/]+)"\),\s*)?'
            r'ElysiumElements\.(\w+)', source):
        name, tag, element = match.group(1), match.group(2), match.group(3)
        found.append((name, ELEMENT_GLOW.get(element, "inert"), True,
                      "c:" + (tag or "ingots/" + name)))

    # The modded table.
    for match in re.finditer(
            r'\{"(\w+)",\s*ElysiumElements\.(\w+),', source):
        name, element = match.group(1), match.group(2)
        found.append((name, ELEMENT_GLOW.get(element, "inert"), True,
                      "c:ingots/" + name))

    # Order-preserving de-duplication: a name registered twice is a bug in the
    # Java, and reporting it here beats generating one set of files for it.
    seen, unique = set(), []
    for entry in found:
        if entry[0] in seen:
            raise SystemExit("material %r is declared twice in ElysiumMaterials.java"
                             % entry[0])
        seen.add(entry[0])
        unique.append(entry)
    return unique


# ---------------------------------------------------------------------------
# Writers
# ---------------------------------------------------------------------------

def write_sprites(entries):
    """A sprite per material and shape, skipping anything already drawn."""
    style.METAL.update(material_art.all_ramps())
    written = 0
    for name, glow, has_armour, _tag in entries:
        metal = name if name in style.METAL else "voidsteel"

        for shape in SHAPES:
            path = os.path.join(TEXTURES, "item", "%s_%s.png" % (name, shape))
            if os.path.exists(path):
                continue     # hand-drawn; leave it alone
            getattr(sprites, shape)(metal, glow).save(path)
            written += 1

        if not has_armour:
            continue
        for piece in ARMOUR:
            path = os.path.join(TEXTURES, "item", "%s_%s.png" % (name, piece))
            if os.path.exists(path):
                continue
            getattr(sprites, piece)(metal, glow).save(path)
            written += 1

        # Worn armour layers, which are what the model on the player uses.
        for index, builder in ((1, layers.layer_one), (2, layers.layer_two)):
            path = os.path.join(TEXTURES, "models/armor",
                                "%s_layer_%d.png" % (name, index))
            if os.path.exists(path):
                continue
            builder(metal, glow).save(path)
            written += 1
    return written


def write_models(entries):
    os.makedirs(MODELS, exist_ok=True)
    written = 0
    for name, _glow, has_armour, _tag in entries:
        pieces = list(SHAPES) + (ARMOUR if has_armour else [])
        for piece in pieces:
            item = "%s_%s" % (name, piece)
            path = os.path.join(MODELS, item + ".json")
            if os.path.exists(path):
                continue
            # handheld for tools, generated for armour: the same distinction
            # vanilla draws, and getting it wrong makes a hammer float flat in
            # the hand.
            parent = ("minecraft:item/handheld" if piece in SHAPES
                      else "minecraft:item/generated")
            json.dump({"parent": parent,
                       "textures": {"layer0": "elysium:item/" + item}},
                      open(path, "w", encoding="utf-8"), indent=2)
            written += 1
    return written


# The crafting patterns. Tools use the same shapes the Elysium gear already
# uses, so a player who has made one hammer knows how to make all of them.
PATTERNS = {
    "hammer":     ["MMM", "MSM", " S "],
    "broadaxe":   ["MMM", "MSM", " S "],
    "scythe":     ["MMM", "  S", "  S"],
    "spear":      ["  M", " S ", "S  "],
    "helmet":     ["MMM", "M M"],
    "chestplate": ["M M", "MMM", "MMM"],
    "leggings":   ["MMM", "M M", "M M"],
    "boots":      ["M M", "M M"],
}


def write_recipes(entries):
    os.makedirs(RECIPES, exist_ok=True)
    written = 0
    for name, _glow, has_armour, tag in entries:
        pieces = list(SHAPES) + (ARMOUR if has_armour else [])
        for piece in pieces:
            item = "%s_%s" % (name, piece)
            path = os.path.join(RECIPES, item + ".json")
            if os.path.exists(path):
                continue
            key = {"M": tag}
            if "S" in "".join(PATTERNS[piece]):
                key["S"] = "minecraft:stick"
            json.dump({
                "type": "minecraft:crafting_shaped",
                "category": "equipment",
                "pattern": PATTERNS[piece],
                "key": key,
                "result": {"id": "elysium:" + item, "count": 1},
            }, open(path, "w", encoding="utf-8"), indent=2)
            written += 1
    return written


def write_lang(entries):
    lang = json.load(open(LANG, encoding="utf-8"))
    titles = {"hammer": "Hammer", "broadaxe": "Broadaxe", "scythe": "Scythe",
              "spear": "Spear", "helmet": "Helmet", "chestplate": "Chestplate",
              "leggings": "Leggings", "boots": "Boots"}
    added = 0
    for name, _glow, has_armour, _tag in entries:
        display = " ".join(word.capitalize() for word in name.split("_"))
        for piece in list(SHAPES) + (ARMOUR if has_armour else []):
            key = "item.elysium.%s_%s" % (name, piece)
            if key in lang:
                continue
            lang[key] = "%s %s" % (display, titles[piece])
            added += 1
    json.dump(lang, open(LANG, "w", encoding="utf-8"),
              indent=2, ensure_ascii=False, sort_keys=True)
    return added


def main():
    entries = read_materials()
    print("materials read     : %d" % len(entries))
    print("sprites written    : %d" % write_sprites(entries))
    print("models written     : %d" % write_models(entries))
    print("recipes written    : %d" % write_recipes(entries))
    print("lang keys added    : %d" % write_lang(entries))


if __name__ == "__main__":
    main()
