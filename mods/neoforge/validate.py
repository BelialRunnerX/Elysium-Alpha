#!/usr/bin/env python3
"""
Cross-check the registry declared in Java against the shipped resources.

Run from the repo root:   python3 validate.py

Fails if a registered item has no model, a model points at a texture that does
not exist, a recipe or loot table references an unregistered item, a shaped
pattern disagrees with its declared keys, a placed feature is unreachable from
any biome modifier, or a data pack folder is still using its pre-1.21 name.
""" 
import json
import os
import re
import sys

WORK = os.path.dirname(os.path.abspath(__file__))
RES = os.path.join(WORK, "src/main/resources")
JAVA = os.path.join(WORK, "src/main/java/com/elysium/Elysium.java")

problems = []
notes = []


def fail(msg):
    problems.append(msg)


def exists(rel):
    return os.path.exists(os.path.join(RES, rel))


source = open(JAVA, encoding="utf-8").read()

blocks = re.findall(r'BLOCKS\.register\("([a-z_]+)"', source)

# Items are registered through a family of small helpers rather than one call,
# so match the name literal wherever it is handed to one of them.
# The leading (?<!\.) keeps qualified calls out - CREATIVE_TABS.register and
# BLOCKS.register are not item registrations.
ITEM_HELPERS = r'(?<!\.)\b(?:register|item|simpleItem|armour|neutroniumArmour|weapon|tool)'
items = re.findall(ITEM_HELPERS + r'\(\s*"([a-z_]+)"', source)

# blockItem() derives the item name from the block's registry name.
items = set(items) | set(blocks)

# Runes are registered from the enum, so their names come from there.
RUNES = os.path.join(WORK, "src/main/java/com/elysium/item/ElysiumRuneItem.java")
rune_source = open(RUNES, encoding="utf-8").read()
rune_ids = re.findall(r'^\s+[A-Z_]+\("([a-z_]+)",', rune_source, re.MULTILINE)
if not rune_ids:
    fail("could not read any rune ids out of ElysiumRuneItem - check the enum format")
items |= {rune_id + "_rune" for rune_id in rune_ids}

items = sorted(items)

print(f"blocks registered : {len(blocks)}")
print(f"items registered  : {len(items)}")

lang = json.load(open(os.path.join(RES, "assets/elysium/lang/en_us.json"), encoding="utf-8"))

# --- models, textures, lang -------------------------------------------------
for block in blocks:
    if not exists(f"assets/elysium/blockstates/{block}.json"):
        fail(f"missing blockstate: {block}")
    if not exists(f"assets/elysium/models/block/{block}.json"):
        fail(f"missing block model: {block}")
    if not exists(f"assets/elysium/textures/block/{block}.png"):
        fail(f"missing block texture: {block}")
    if f"block.elysium.{block}" not in lang:
        fail(f"missing lang key: block.elysium.{block}")
    if not exists(f"data/elysium/loot_table/blocks/{block}.json"):
        fail(f"missing loot table: {block}")

for item in items:
    if not exists(f"assets/elysium/models/item/{item}.json"):
        fail(f"missing item model: {item}")
    key = f"block.elysium.{item}" if item in blocks else f"item.elysium.{item}"
    if key not in lang:
        fail(f"missing lang key: {key}")

# --- every model's texture reference resolves -------------------------------
model_root = os.path.join(RES, "assets/elysium/models")
for dirpath, _, filenames in os.walk(model_root):
    for filename in filenames:
        path = os.path.join(dirpath, filename)
        model = json.load(open(path, encoding="utf-8"))
        for slot, ref in (model.get("textures") or {}).items():
            if not ref.startswith("elysium:"):
                continue
            rel = "assets/elysium/textures/" + ref.split(":", 1)[1] + ".png"
            if not exists(rel):
                fail(f"{os.path.relpath(path, RES)}: texture not found -> {ref}")
        parent = model.get("parent", "")
        if parent.startswith("elysium:"):
            rel = "assets/elysium/models/" + parent.split(":", 1)[1] + ".json"
            if not exists(rel):
                fail(f"{os.path.relpath(path, RES)}: parent model not found -> {parent}")

# --- armour layer textures --------------------------------------------------
for material in re.findall(r'ResourceLocation\.fromNamespaceAndPath\(Elysium\.MODID, "(\w+)"\)\)\)',
                           open(os.path.join(WORK, "src/main/java/com/elysium/item/ElysiumArmorMaterials.java"),
                                encoding="utf-8").read()):
    for layer in (1, 2):
        rel = f"assets/elysium/textures/models/armor/{material}_layer_{layer}.png"
        if not exists(rel):
            fail(f"missing armour layer texture: {rel}")

# --- GUI texture referenced by the screen -----------------------------------
if not exists("assets/elysium/textures/gui/reforge_table.png"):
    fail("missing GUI texture: textures/gui/reforge_table.png")

# --- recipes / loot tables only reference things that exist -----------------
known_items = {f"elysium:{name}" for name in items}
VANILLA_OK = re.compile(r"^(minecraft|#minecraft|c|#c):")


def check_id(value, where):
    if not isinstance(value, str):
        return
    if VANILLA_OK.match(value):
        return
    if value.startswith("elysium:") and value not in known_items:
        fail(f"{where}: references unknown item -> {value}")


for dirpath, _, filenames in os.walk(os.path.join(RES, "data/elysium/recipe")):
    for filename in filenames:
        path = os.path.join(dirpath, filename)
        recipe = json.load(open(path, encoding="utf-8"))
        where = os.path.relpath(path, RES)
        check_id(recipe.get("result", {}).get("id"), where)
        for value in (recipe.get("key") or {}).values():
            check_id(value, where)
        for value in (recipe.get("ingredients") or []):
            check_id(value, where)
        if recipe["type"] == "minecraft:crafting_shaped":
            widths = {len(row) for row in recipe["pattern"]}
            if len(widths) != 1:
                fail(f"{where}: pattern rows have differing widths {widths}")
            used = {c for row in recipe["pattern"] for c in row if c != " "}
            declared = set(recipe["key"])
            if used - declared:
                fail(f"{where}: pattern uses undeclared key(s) {used - declared}")
            if declared - used:
                fail(f"{where}: declares unused key(s) {declared - used}")


def walk_json(node, where):
    if isinstance(node, dict):
        for key, value in node.items():
            if key == "name" and isinstance(value, str):
                check_id(value, where)
            else:
                walk_json(value, where)
    elif isinstance(node, list):
        for value in node:
            walk_json(value, where)


for dirpath, _, filenames in os.walk(os.path.join(RES, "data/elysium/loot_table")):
    for filename in filenames:
        path = os.path.join(dirpath, filename)
        walk_json(json.load(open(path, encoding="utf-8")), os.path.relpath(path, RES))

# --- worldgen wiring --------------------------------------------------------
placed_dir = os.path.join(RES, "data/elysium/worldgen/placed_feature")
for filename in os.listdir(placed_dir):
    path = os.path.join(placed_dir, filename)
    placed = json.load(open(path, encoding="utf-8"))
    if "feature" not in placed:
        fail(f"placed_feature/{filename}: missing 'feature' field")
    else:
        target = placed["feature"].split(":", 1)[1]
        if not exists(f"data/elysium/worldgen/configured_feature/{target}.json"):
            fail(f"placed_feature/{filename}: points at missing configured feature {placed['feature']}")

bm_dir = os.path.join(RES, "data/elysium/neoforge/biome_modifier")
referenced = set()
for filename in os.listdir(bm_dir):
    modifier = json.load(open(os.path.join(bm_dir, filename), encoding="utf-8"))
    referenced.add(modifier["features"])
for filename in os.listdir(placed_dir):
    feature = "elysium:" + filename[:-5]
    if feature not in referenced:
        fail(f"placed feature {feature} is never added to a biome by any biome modifier")

# --- obtainability: every item has a path back to the world ------------------
#
# The check that matters most and is easiest to lose. An item can have a model,
# a texture, a lang key and a working class and still be unreachable in
# survival, which is exactly what happened to five armour pieces: they were
# registered, rendered, socketable and reforgeable, and there was no way to
# get one without creative mode.
#
# So: start from what the world itself gives you, then close over the recipe
# graph until nothing new appears. Anything still unreached is either a bug or
# an item that must be declared creative-only on purpose.

# Deliberately creative-only. Spawn eggs are a creative tool by definition.
CREATIVE_ONLY = {"imperial_enforcer_spawn_egg", "unsworn_raider_spawn_egg"}

# Ore blocks are reachable because they generate. Everything else has to be
# earned through the graph below.
worldgen_blocks = {name[:-5] for name in os.listdir(placed_dir)}

obtainable = set()
sources = {}


def source(name, why):
    if name not in obtainable:
        obtainable.add(name)
        sources[name] = why


# 1. Blocks that generate, and whatever their loot tables drop.
for block in sorted(worldgen_blocks):
    source(block, "worldgen")
    table = f"data/elysium/loot_table/blocks/{block}.json"
    if not exists(os.path.join(RES, table)) and not exists(table):
        fail(f"{block} generates in the world but has no loot table - it would drop nothing")
        continue
    text = open(os.path.join(RES, table), encoding="utf-8").read()
    for dropped in re.findall(r'"elysium:([a-z_]+)"', text):
        source(dropped, f"mined from {block}")

# 2. Whatever the mobs drop, by data.
entity_dir = os.path.join(RES, "data/elysium/loot_table/entities")
for filename in os.listdir(entity_dir):
    text = open(os.path.join(entity_dir, filename), encoding="utf-8").read()
    for dropped in re.findall(r'"elysium:([a-z_]+)"', text):
        source(dropped, f"dropped by {filename[:-5]}")

# 3. Whatever the mod hands out in code, from any handler that does so.
#    Listed by name rather than discovered, so adding a new giver is a
#    deliberate act and a typo in a path fails loudly instead of quietly
#    marking an item unobtainable.
CODE_GIVERS = [
    ("src/main/java/com/elysium/event/ElysiumLootHandler.java", "standing loot"),
    ("src/main/java/com/elysium/event/ElysiumJoinHandler.java", "granted on first join"),
]
handler = ""
for _path, _why in CODE_GIVERS:
    _full = os.path.join(WORK, _path)
    if not os.path.exists(_full):
        fail(f"obtainability audit points at a missing file: {_path}")
        continue
    handler += open(_full, encoding="utf-8").read()
holders = dict(re.findall(
    r'public static final DeferredHolder<Item, Item> ([A-Z0-9_]+)\s*=\s*\n?\s*'
    r'(?:register|item|simpleItem|armour|neutroniumArmour|weapon|tool)\(\s*"([a-z_]+)"',
    source_java := open(JAVA, encoding="utf-8").read()))
for holder in re.findall(r'Elysium\.([A-Z0-9_]+)\.get\(\)', handler):
    if holder in holders:
        source(holders[holder], "standing loot")
if not any(why == "standing loot" for why in sources.values()):
    fail("could not read any rewards out of ElysiumLootHandler - check the Elysium.X.get() form")

# 4. Close over the recipe graph: a recipe is reachable once every one of its
#    Elysium ingredients is. Repeat until nothing new appears, which is also
#    what catches a circular recipe - it simply never becomes reachable.
recipes = []
for filename in os.listdir(os.path.join(RES, "data/elysium/recipe")):
    recipe = json.load(open(os.path.join(RES, "data/elysium/recipe", filename), encoding="utf-8"))
    result = recipe["result"]["id"].split(":", 1)[1]
    ingredients = set()
    for value in list((recipe.get("key") or {}).values()) + list(recipe.get("ingredients") or []):
        if isinstance(value, str) and value.startswith("elysium:"):
            ingredients.add(value.split(":", 1)[1])
    recipes.append((filename, result, ingredients))

changed = True
while changed:
    changed = False
    for filename, result, ingredients in recipes:
        if result not in obtainable and ingredients <= obtainable:
            source(result, f"crafted ({filename[:-5]})")
            changed = True

unreachable = sorted(set(items) - obtainable - CREATIVE_ONLY)
for name in unreachable:
    made_by = [f for f, result, _ in recipes if result == name]
    if made_by:
        blocking = set()
        for filename, result, ingredients in recipes:
            if result == name:
                blocking |= ingredients - obtainable
        fail(f"{name} is craftable but unreachable - needs {sorted(blocking)}, "
             f"which nothing in the world produces")
    else:
        fail(f"{name} has no survival source: no recipe, no loot table, no code drop")

print(f"obtainable in survival : {len(obtainable & set(items))}/{len(set(items) - CREATIVE_ONLY)}"
      f"  (+{len(CREATIVE_ONLY)} creative-only by design)")

# --- character balance invariants -------------------------------------------
#
# These are properties the code documents about itself and cannot check at
# runtime. Both were violated in the first draft and both looked fine while
# they were: race bases ranged 41 to 47 while claiming to be equal, and every
# class gave three growth points while claiming to give two — which quietly
# made class growth equal to race growth, the opposite of the stated design.

CHARACTER = os.path.join(WORK, "src/main/java/com/elysium/character")


def stat_blocks(path):
    """Every ElysiumStatBlock.of(...) in a file, as a list of totals."""
    text = open(path, encoding="utf-8").read()
    blocks = []
    for match in re.finditer(r"ElysiumStatBlock\.of\((.*?)\)", text, re.S):
        numbers = re.findall(r",\s*(\d+)", match.group(1))
        blocks.append(sum(int(value) for value in numbers))
    return blocks


race_blocks = stat_blocks(os.path.join(CHARACTER, "ElysiumRace.java"))
if len(race_blocks) != 12:
    fail(f"expected 12 stat blocks in ElysiumRace (6 races x base+growth), "
         f"found {len(race_blocks)} - the balance check cannot read the file")
else:
    bases = race_blocks[0::2]
    growths = race_blocks[1::2]
    if len(set(bases)) != 1:
        fail(f"race starting blocks are not equal: {bases} - every race must "
             f"begin with the same number of points")
    if set(growths) != {3}:
        fail(f"race growth must be 3 points per level, found {growths}")
    print(f"race balance       : {len(bases)} races at {bases[0]} base, "
          f"{growths[0]}/level growth")

class_blocks = stat_blocks(os.path.join(CHARACTER, "ElysiumClass.java"))
if not class_blocks:
    fail("could not read any stat blocks out of ElysiumClass")
elif set(class_blocks) != {2}:
    fail(f"class growth must be 2 points per level - less than a race's 3 - "
         f"found {class_blocks}")
else:
    print(f"class balance      : {len(class_blocks)} classes at "
          f"{class_blocks[0]}/level growth")

# --- data pack folder names must be the 1.21 singular form ------------------
for legacy in ["data/elysium/recipes", "data/elysium/loot_tables",
               "data/minecraft/tags/blocks", "data/minecraft/tags/items"]:
    if exists(legacy):
        fail(f"pre-1.21 data pack folder still present: {legacy}")

# --- every block is mineable ------------------------------------------------
pickaxe = json.load(open(os.path.join(RES, "data/minecraft/tags/block/mineable/pickaxe.json"),
                         encoding="utf-8"))["values"]
for block in blocks:
    if f"elysium:{block}" not in pickaxe:
        fail(f"{block} requires a correct tool but is in no mineable tag - it would drop nothing")

# --- all JSON parses --------------------------------------------------------
count = 0
for dirpath, _, filenames in os.walk(RES):
    for filename in filenames:
        if filename.endswith(".json"):
            count += 1
            try:
                json.load(open(os.path.join(dirpath, filename), encoding="utf-8"))
            except Exception as exc:
                fail(f"invalid JSON {os.path.join(dirpath, filename)}: {exc}")

print(f"json files parsed : {count}")
print()

if problems:
    print(f"{len(problems)} PROBLEM(S):")
    for problem in problems:
        print("  -", problem)
    sys.exit(1)

print("all resource checks passed")
