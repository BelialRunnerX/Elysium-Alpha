# Port and fix log

The project was written against **Forge 1.20.x** APIs while declaring
**NeoForge 1.21.1**. Nothing in `src/main/java` compiled. This is what changed.

---

## 1. Hard compile failures

| Where | Problem |
|---|---|
| `Elysium.java` | `ForgeRegistries`, `RegistryObject`, `MinecraftForge.EVENT_BUS`, `FMLJavaModLoadingContext` — none exist on NeoForge. Replaced with `BuiltInRegistries`, `DeferredHolder`, and the `IEventBus` mod constructor. |
| `Elysium.java` | `FMLLoader.get().isLoaded(...)` is not a method. Now `ModList.get().isLoaded(...)`. |
| `Elysium.java` | Referenced `REFORGE_TABLE_MENU` and `REFORGE_TABLE_BLOCK_ENTITY`, neither of which was ever declared. Both are now registered. |
| `ElysiumAffix`, `ElysiumPsionicAffix`, `ElysiumAffixes` | Imported `shadows.apotheosis.*`, which was not a dependency in `build.gradle` — unbuildable with or without Apotheosis installed. Rewritten as a self-contained affix system. |
| `ElysiumSilentGear`, `ElysiumLegendaryTooltips` | Called `ModList` while importing `FMLLoader`. Import fixed. |
| `ReforgeTableMenu` | Imported `net.minecraftforge.items.*`. NeoForge's package is `net.neoforged.neoforge.items.*`. |
| `ElysiumArmorMaterials`, `NeutroniumArmorMaterials` | Implemented `ArmorMaterial` as an interface. In 1.21.1 it is a **registered record**. Both are now registered materials referenced through a `Holder`. |
| `ElysiumRarities` | `Rarity.create(...)` was a Forge extension that no longer exists; `Rarity` is a plain four-value enum. Tiers now map to vanilla rarities, with the tier itself shown in the tooltip. |
| `ElysiumArmorItem`, `ElysiumReforgeItem` | Overrode `getRarity(ItemStack)`, removed in 1.20.5. Rarity is set via `Item.Properties#rarity`. |
| `ElysiumArmorItem`, `ElysiumReforgeHandler`, `ElysiumArmorAscension` | Used `stack.getTag()` / `getOrCreateTag()` / `setTag()` / `hasTag()`, all removed in 1.20.5. Replaced by a registered data component. |
| `ElysiumArmorItem`, `ElysiumPsionicAffix` | `new AttributeModifier(UUID, String, double, Operation)` and `Operation.ADDITION` / `MULTIPLY_BASE`. 1.21 uses `(ResourceLocation, double, Operation)` with `ADD_VALUE` / `ADD_MULTIPLIED_BASE`. |
| `ReforgeTableScreen`, `ElysiumRecipeProvider` | `new ResourceLocation(...)` is private in 1.21. Now `ResourceLocation.fromNamespaceAndPath(...)`. |
| `ElysiumRecipeProvider` | `RecipeProvider(DataGenerator)` and `buildRecipes(Consumer<FinishedRecipe>)` are both 1.20 signatures. Now `(PackOutput, CompletableFuture<HolderLookup.Provider>)` and `buildRecipes(RecipeOutput)`, with the `RecipeCategory` argument the builders have required since 1.19. |
| `ElysiumBlockStateProvider` | Constructor took a `DataGenerator`; 1.21.1 takes a `PackOutput`. |
| `DataGenerators` | `@Mod.EventBusSubscriber` moved to the top-level `net.neoforged.fml.common.EventBusSubscriber`. |
| `ReforgeTableBlockEntity` | `saveAdditional(CompoundTag)` / `load(CompoundTag)` now take a `HolderLookup.Provider`. |
| `Elysium.java` | `event.accept(DEFERRED_HOLDER)` does not compile — the creative tab event takes an `ItemLike`, and a plain `DeferredHolder` is not one. Now resolves with `.get()`. |

## 2. Logic bugs that would have shipped broken

- **Reforge bonuses stacked without limit.** `applyReforgedStats` added transient
  attribute modifiers with `UUID.randomUUID()` on every armour tick, so a worn
  piece kept piling on fresh copies forever. Bonuses are now part of the item's
  attribute modifiers, computed from the stack.
- **`onArmorTick` never ran.** NeoForge removed it in 1.21. The rune effects it
  drove are now applied from a `PlayerTickEvent.Post` handler.
- **Reforging could crash.** `RANDOM.nextInt(finalPoints / 3)` throws when the
  argument is 0, reachable at low tier with a sub-1.0 grade multiplier. Guarded.
- **Reforge quality read the wrong thing.** `getRarityMultiplier` used
  `Rarity#ordinal()`, which silently became meaningless once the custom rarities
  were gone. It now reads the tier.
- **Ascension dropped data.** It rebuilt a bare stack and re-attached a copied
  tag, losing anything not explicitly carried across. It now copies the stack.
- **The Reforge Table could never open.** `ReforgeTableBlock` extended plain
  `Block`, so its block entity was never created and there was no interaction
  handler. It now implements `EntityBlock` and opens the menu.
- **No screen was registered.** Opening the menu server-side with no client
  screen registered disconnects the player. Registered via
  `RegisterMenuScreensEvent`.
- **The Reforge button did nothing** except close the screen. It now sends a
  container button click, handled in `clickMenuButton`.
- **The workstation lost its contents.** The `ItemStackHandler` lived on the
  menu, so anything left in the table vanished when the screen closed. It now
  lives on the block entity and is saved to NBT.
- **`quickMoveStack` returned `ItemStack.EMPTY` unconditionally**, which
  desyncs the client and server on shift-click. Implemented properly.
- **Five blocks were never registered** — `AetheriumOreBlock`,
  `VoidglassOreBlock`, `ReforgeTableBlock`, `RuneSocketTableBlock`,
  `AscensionForgeBlock`. All seven blocks are now registered with items, models,
  loot tables and recipes.
- **`NeutroniumArmorMaterials` was dead code** — defined but never used, while
  the recipes and loot tables referenced a neutronium armour set that did not
  exist. The set is now registered.
- **`aetherium_ingot` and `voidglass_ingot` did not exist** but were the drops
  of two ore loot tables. Both are now items.

## 3. Resources

- **Data pack folders were pre-1.21.** `recipes/` → `recipe/`,
  `loot_tables/` → `loot_table/`, `tags/blocks/` → `tags/block/`. Under the old
  names nothing loads at all.
- **Recipe JSON was pre-1.21.** Results use `"id"`, not `"item"`; ingredients are
  bare id strings.
- **Recipes pointed at items that did not exist** (`elysium:neutronium_helmet`
  and friends) — a datapack error on load. All recipes now resolve.
- **Rune-socket and ascension crafting recipes were removed.** A crafting grid
  cannot preserve item components: the player fed in a socketed, reforged piece
  and got a blank one back. Both operations live on the workstation blocks, where
  the data survives.
- **Every `placed_feature` was malformed** — they restated the ore config inline
  and omitted the required `feature` field, so all three failed to parse.
- **No biome modifiers existed**, so even a valid placed feature would never
  have been added to a biome. None of the ores generated. Added under
  `data/elysium/neoforge/biome_modifier/`.
- **No `mineable/pickaxe` tag.** Every block called `requiresCorrectToolForDrops()`
  but sat in no mineable tag, so none of them dropped anything. Added, along with
  tiered `needs_*_tool` tags.
- **The lang file was the untouched MDK template** — "Example Block", "Example
  Item", and config keys for a config that does not exist. Not one real item was
  named. Rewritten.
- **Six item models were missing** (the catalyst, the crown and four runes), so
  those items rendered as the missing-model cube.
- **Both `mods.toml` and `neoforge.mods.toml` were present.** NeoForge reads
  `neoforge.mods.toml`; the real metadata was sitting in the ignored one, while
  the one that counts still said "Example mod description". The stale file is
  deleted and the metadata moved across.
- **`loaderVersion="[52,)"`** in `mods.toml` is not an FML version and would have
  refused to load. Now `[1,)`.

## 4. Textures

Every texture in the project was a **16×16 square of one flat colour with no
alpha channel** — items would have rendered as solid blocks. On top of that, the
two armour layer textures were 16×16; they must be 64×32 to map onto the player
model, so worn armour would have rendered as garbage.

The whole set is now drawn to a defined art direction — see `TEXTURES.md`,
which is a standing objective rather than a one-off pass. 30 textures, no
placeholders, generated reproducibly from `tools/textures/`.

- Item sprites are shaded silhouettes with real transparency and a near-black
  outline, one focal emissive accent each.
- Rune sigils are *carved* — a dark recess cut into the tablet with a lit line
  along the bottom of the cut — rather than a glow painted on top.
- Armour layers are painted region by region against the humanoid UV map:
  helmet on the head box, chestplate on the body and arm boxes, boots on the
  lower leg box in layer 1, leggings on the leg box plus the waist in layer 2.
  `tools/textures/preview.py` composites the front faces onto a player figure
  so this can actually be checked.
- Ore blocks are crystal veins bedded into a clustered stone matrix, not
  per-pixel static.
- Added: neutronium armour layers, three workstation block textures, two
  ingots, four armour icons, and the workstation GUI — which the screen
  referenced but which never existed.

## 5. Build

- **`processResources` token expansion was missing.** `neoforge.mods.toml` is
  full of `${mod_id}`-style placeholders and nothing expanded them, so the jar
  would have shipped the literal text and failed to load.
- **`settings.gradle` had no `rootProject.name`** and no foojay toolchain
  resolver, so Gradle could not auto-provision a JDK 21.
- **`libs/parchment-1.21.1-….zip` plus a `flatDir` repository** is not how
  NeoGradle consumes Parchment. Removed; the (commented) `neogradle.subsystems`
  properties are the real switch.
- **`mavenLocal()`** removed from the repositories.
- Added run configurations (`runClient` / `runServer` / `runData`), a
  `localRuntime` configuration, `duplicatesStrategy` so `runData` cannot break a
  later `build`, and raised the Gradle heap from 1G to 2G.
- **CI now uploads the jar.** The workflow only ran `./gradlew build` and threw
  the output away. It now attaches `elysium-jar` to the run, plus build reports
  on failure, and can be triggered by hand.

## 5b. Area tools and rune alignment

Added after the first pass, and it exposed two real bugs in the existing code.

- **`ElysiumAffix.createModifier` produced colliding modifier ids.** Modifiers
  are keyed by `ResourceLocation` in 1.21, not a random UUID. Two armour pieces
  carrying the same rune therefore produced two modifiers with identical ids on
  the same attribute, and only one of them survived — a four-piece set was
  quietly giving one piece's worth of rune bonus. The id now carries the
  `EquipmentSlotGroup`. The same fix applies to the three reforge modifiers.
- **Socketing was armour-only**, which made rune alignment a half-idea: a
  Voidglass hammer with a Void affinity had nothing to align *with*. Sockets
  now live on one `ElysiumSocketable` interface implemented by armour, weapons
  and tools alike, backed by shared logic in `ElysiumSockets`. The Rune Socket
  Table's first slot accepts any of them; reforging and ascension stay
  armour-only and simply do nothing for the rest.

New behaviour:

- `ElysiumAreaBreak` — a 3×3 in the plane of the struck face, and a bounded
  flood fill up a tree (192 logs, searching `dy 0..1` so a fell takes the tree
  rather than tunnelling into the forest floor). Both are guarded against
  re-entry.
- Durability for extra blocks is spent by hand rather than through
  `hurtAndBreak`, whose signature could not be verified against this exact
  build; the tool refuses its last point rather than snapping mid-swing. The
  tradeoff is that Unbreaking does not apply to the extra blocks — swap the
  helper in once the signature can be checked against the real jar.
- The four tool classes extend the concrete `PickaxeItem` / `AxeItem` /
  `ShovelItem` / `HoeItem` rather than `DiggerItem`, and build their attack
  attributes from `ItemAttributeModifiers.builder()` rather than the vanilla
  `createAttributes` helpers — both choices made because those signatures could
  not be verified this session, and both restricted to API this project has
  already checked.
- Held Elysium gear now counts in the counter matrix and toward utility runes,
  so a hammer is a weapon in every system, not only in its damage number.

## 5c. Obtainability

An audit of whether the mod can actually be played without creative mode. It
found one real hole and one soft-lock.

- **Five armour pieces had no survival source.** The Elysium Helm, Plasma
  Carapace, Neural Leggings, Dimensional Boots and Emperor's Crown had no
  recipe and appeared in no loot table. Every one of them was registered,
  rendered, socketable, reforgeable and ascendable — which meant the entire
  defensive half of the counter matrix, plus reforging and ascension, was
  creative-only, because all three systems operate on Elysium armour. The four
  elemental pieces now craft from their material plus the rune of their
  element, the same bargain the weapons make. The Crown drops from a named
  Imperial Enforcer at 4%, and only at Hunted.
- **Favor could not be climbed out of.** Decay took a point every two minutes
  from both meters including at the bottom of the range, while an incidental
  kill was worth one point one time in five. That is a net loss at any
  realistic pace, so the first rung — Recognised, which is what turns on faction
  spawns and the loot table — was unreachable outside a mob farm. Decay now
  stops at the notice threshold, and an incidental kill pays one time in three.
  Above notice nothing changes: standing is still something you hold by
  continuing to act.

`validate.py` now proves the property rather than trusting it. It starts from
what the world gives you — ores that generate, block loot, mob loot, and the
rewards the standing handler hands out in code — then closes over the recipe
graph until nothing new appears. Anything unreached fails the build, and a
circular recipe fails it too, since a cycle simply never becomes reachable.
Two spawn eggs are declared creative-only by name; everything else must earn
its place.

Two paths deliberately cross so neither meter can dead-end the other:
Neutronium ore needs a netherite pickaxe, but Neutronium ingots also come off
Enforcers and out of the Favor table; and Suspicion bootstraps from mining
Voidglass or Aetherium, which need only iron, so you never need an Empire mob
in order to anger the Empire.

## 5d. Characters: stats, races, classes

The largest addition so far, and the one that most needed the API checked
rather than guessed. Everything client-facing here was verified against the
1.21.1 javadocs and the NeoForge networking documentation before a line of it
was written: `CustomPacketPayload.Type` and `StreamCodec.composite`,
`PayloadRegistrar#playToClient`/`playToServer`, `IPayloadContext`,
`PacketDistributor.sendToPlayer`, `PlayerEvent.PlayerLoggedInEvent`,
`Screen#init`/`render`/`addRenderableWidget`, `Button.Builder#bounds`,
`GuiGraphics#drawString`/`drawCenteredString`/`drawWordWrap`/`fill`, and
`AttributeInstance#addOrUpdateTransientModifier`/`removeModifier`.

Decisions worth recording:

- **Every proportional stat reads through `v/(v+K)`.** Reforging and ascension
  are meant to climb forever, so a percentage stat must accept any input
  without reaching 100%. The curve rises fast, flattens, and approaches its
  ceiling without arriving — so gear can grant arbitrary points and nothing
  ever divides by zero or turns a player invulnerable.
- **Attribute modifiers are transient and re-derived once a second.** A
  permanent modifier would have to be removed exactly as often as it is added,
  which is how a save ends up with four hundred stacked copies of one armour
  bonus. Transient modifiers are not saved at all, so the worst failure is a
  value that is stale for under a second.
- **The character sheet travels as one packed string.** A fifteen-field packet
  has to agree on field order between a client and a server that ship
  independently, and gains a field every time a stat does. One string survives
  a stat being added or removed, and every payload in the mod is the exact
  two-field `StreamCodec.composite` shape the documentation shows.
- **The client handler is a lambda, not a method reference.** Registration runs
  on a dedicated server too, and a direct reference to a client-only class
  would be resolved there and crash on startup. A lambda body is not resolved
  until it runs, and a client-bound packet only runs on a client.
- **Nothing a client sends is trusted.** Race and class arrive as strings and
  are looked up against the enum; anything that does not resolve is dropped.
  Point spending is clamped against the balance the server holds, never the
  number the packet supplied. Race is refused outright if one is already set,
  so a modified client cannot reroll its biology.
- **Damage reflection is guarded three ways** — living attacker only, never
  self, and dealt through a thorns source so the other party cannot reflect it
  back. A reflection loop is the classic way to freeze a server.
- **Resilience is applied last**, after the elemental matrix, so a defender's
  percentage answers the blow that is actually arriving rather than the number
  it started as.

Caps removed, all of which existed only because nothing had asked them to
climb: the tier ceiling at Unique, the three-socket ceiling, the flat
three-reforges-per-piece (now three per tier, refilled by ascension), and the
elemental advantage table's last entry.

## 5e. What the review of 5d found

The character system was reviewed adversarially after it compiled, on the basis
that "it compiles and the API is real" says nothing about whether the rules are
right. Fourteen findings; the ones that mattered:

- **Reflection re-entered the combat handler and was amplified by the
  defender's own melee stats.** `DamageSources.thorns(defender)` sets the
  defender as both the causing *and* the direct entity, so the reflected packet
  came back through `onIncomingDamage`, satisfied the melee check, and picked up
  the defender's Strength, weapon multiplier, elemental advantage and a critical
  roll. A 0.3-damage reflection became ~180. The guard had been placed around
  the reflection step; it needed to be around the whole handler.
- **Every reforge was paid twice.** The roll became character stats *and* stayed
  as the old attribute modifiers on the item, hitting the same three attributes
  from two directions, with only half of it visible on the tooltip.
- **A client could change class between one swing and the next.** `ChooseCharacter`
  honoured a repeat class choice with no cost, cooldown or proximity check —
  Factor before a kill, Warden before a hit, Marksman before a crit. Both halves
  are now set once. Changing class belongs at a workstation with a price, and
  until that exists it is not on offer.
- **Ascension never checked that the two pieces were the same tier.** The cost of
  reaching tier N was N base pieces rather than 2^N, which was the only thing
  keeping unbounded ascension honest.
- **Silk Touch made character levels free.** Break ore, place ore, break ore: six
  experience a cycle against a level track with no ceiling. Player-placed ore is
  now remembered and pays nothing.
- **The reforge budget collapsed the moment a piece went past Unique** — 25
  points at tier 5, 3 at tier 6, because the table had no case above the named
  range and fell through to its default. Exactly backwards.
- **The Imperial passive approached 100% reflection, not the documented half.**
  A missing 0.5 — subsequently resolved the other way: 100% is the intended
  ceiling, so the doc was wrong rather than the code. See 5f.
- **Four class passives were never called at all.** Artificer's durability
  saving and reforge bonus, Reclaimer's second ingot, Psion's rune resonance —
  all defined, none invoked. Three are now wired; Psion's is re-expressed as
  psionic potency, because "an aligned rune counts twice" cannot be implemented
  where rune affixes live: they are baked into an item's attribute component,
  and an item does not know who is holding it.
- **Korrath molt ran while burning.** `getLastHurtByMobTimestamp` is only written
  for damage with a living attacker, and defaults to 0 — so a new player molted
  from tick 101 without ever having been left alone.
- **Two documented balance invariants were false.** Race starting blocks claimed
  to be equal and ranged from 41 to 47; classes claimed 2 growth points a level
  and all gave 3, making class growth equal to race growth rather than less.
  Both are now 44 and 2, and `validate.py` checks both — the checks were tested
  by breaking each one and confirming it failed.

Also fixed: the attacker's elemental advantage read the item's registered tier
while the defender's read the stack's effective tier, so an ascended weapon
would never have scaled; the Codex could be farmed by relogging without
answering the picker; and two dead methods whose doc comments claimed call sites
they did not have — one of which, `counteredBy`, returned the opposite of what
its name said.

## 5f. Reflection stacking

Sanctioned Answer is meant to approach 100%, not half. Restoring that exposed
the real problem, which was never the constant: shares were being **added and
then clamped at 0.95**. Under that scheme an Imperial past level 200 was
saturated, and every point of Retribution after it bought exactly nothing —
the carefully diminishing curves upstream stopped mattering the moment the
clamp bound.

Shares now combine the way overlapping mitigation actually works:

```
total = 1 - (1 - a)(1 - b)
```

Each source contributes what the previous ones let through. 60% and 60% is 84%;
a third 60% makes it 94%. The total approaches 1.0 and cannot reach it, because
a product of factors each strictly below 1 is strictly below 1 — so the ceiling
is a property of the arithmetic rather than a clamp somebody has to remember to
apply. Both inputs are forced into [0, 1) at the combine site, so that holds
even if a future caller passes something out of range.

Verified numerically across extreme inputs (level and Retribution up to 10^9,
combined three times over): worst case 0.999999.

Warden's Bulwark changed shape as a consequence. "Retribution doubles" has no
meaning once shares are proportional — doubling 60% is 120% — so it now lays
the whole share over itself, taking 60% to 84%.

## 6. Verified

The sources compile cleanly against a signature-accurate stub of the 1.21.1 API
surface, every JSON file parses, every model resolves to a texture that exists,
every recipe and loot table resolves to a registered item, every shaped pattern
matches its declared keys, every placed feature is reachable from a biome
modifier, every registered item is reachable in survival from worldgen, mob loot,
a code-driven grant, or a recipe chain that bottoms out in one of those.

That is not the same as a real NeoForge build — run `./gradlew build`, or push
and let CI do it.
