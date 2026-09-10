package com.elysium.event;

import com.elysium.Elysium;
import com.elysium.entity.ElysiumFaction;
import com.elysium.standing.ElysiumStanding;
import net.minecraft.util.RandomSource;
import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.item.ItemEntity;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.Item;
import net.minecraft.world.item.ItemStack;
import net.minecraft.world.level.Level;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.entity.living.LivingDropsEvent;

/**
 * What the two meters are worth, in items.
 *
 * The split is the whole design:
 *
 * <ul>
 *   <li><b>Favor sets the tier.</b> Which shelf the reward comes off — raw
 *       material at the bottom, a rune in the middle, a catalyst or a weapon at
 *       the top.</li>
 *   <li><b>Suspicion sets the amount.</b> How many of it, one through five.</li>
 * </ul>
 *
 * So the two loops pay differently and a player can feel which one they are on.
 * Pure Favor is a trickle of good things. Pure Suspicion is a pile of cheap
 * ones. Both at once is the jackpot, and also four enforcers and four raiders
 * converging on you at the same time.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumLootHandler {

    private ElysiumLootHandler() {
    }

    @SubscribeEvent
    public static void onLivingDrops(LivingDropsEvent event) {
        LivingEntity victim = event.getEntity();
        Level level = victim.level();
        if (level.isClientSide()) {
            return;
        }

        Entity killer = event.getSource().getEntity();
        if (!(killer instanceof Player player)) {
            return;
        }

        ElysiumFaction faction = ElysiumFaction.of(victim);
        if (faction == ElysiumFaction.NEUTRAL) {
            return;
        }

        int favor = ElysiumStanding.getFavor(player);
        int suspicion = ElysiumStanding.getSuspicion(player);
        RandomSource random = player.getRandom();

        // The mod's own faction mobs always pay. Ordinary hostiles roll for it,
        // and only once either meter is above notice.
        if (!ElysiumFaction.isNamedCombatant(victim)
                && random.nextFloat() >= ElysiumStanding.incidentalDropChance(favor, suspicion)) {
            return;
        }

        int tier = ElysiumStanding.lootTier(favor);
        int amount = ElysiumStanding.lootAmount(suspicion);

        // Luck is a chance at more of what Suspicion already decided you get.
        // It multiplies the amount rather than the tier so that the two meters
        // keep their distinct jobs: Favor still owns quality, Suspicion still
        // owns quantity, and Luck simply makes quantity go further.
        float extra = ElysiumPassiveHandler.extraDropChance(player);
        for (int i = 0; i < amount; i++) {
            addDrop(event, victim, level, rollReward(tier, random));
            if (random.nextFloat() < extra) {
                addDrop(event, victim, level, rollReward(tier, random));
            }
        }

        rollCrown(event, victim, level, faction, suspicion, random);
    }

    /**
     * The Emperor's Crown is the one piece with no recipe.
     *
     * It is Elysomnion's own — a bench cannot produce it, so it comes off the
     * body of someone the Empire sent, and only once Suspicion has reached
     * Hunted. That is the top of the loop the Crown belongs to: you get it by
     * being worth sending enforcers after, and by killing enough of them.
     *
     * At {@value #CROWN_CHANCE} per named enforcer this is a chase, not a
     * reward, which is what a Unique-tier item should be. It is also the reason
     * Suspicion decay matters — park the meter and the drop stops.
     */
    private static void rollCrown(LivingDropsEvent event, LivingEntity victim, Level level,
                                  ElysiumFaction faction, int suspicion, RandomSource random) {
        if (faction != ElysiumFaction.EMPIRE
                || !ElysiumFaction.isNamedCombatant(victim)
                || ElysiumStanding.bandOf(suspicion) < ElysiumStanding.BAND_HUNTED
                || random.nextFloat() >= CROWN_CHANCE) {
            return;
        }
        addDrop(event, victim, level, new ItemStack(Elysium.EMPEROR_CROWN.get()));
    }

    /** Per named Imperial Enforcer killed at Hunted. */
    private static final float CROWN_CHANCE = 0.04F;

    /**
     * One item off the shelf Favor has unlocked.
     *
     * Each tier keeps a chance of the tier below it, so the reward still varies
     * once a player is parked at the top — a fixed table stops being a reward
     * and becomes a rate.
     */
    private static ItemStack rollReward(int tier, RandomSource random) {
        return switch (tier) {
            case 3 -> {
                if (random.nextFloat() < 0.12F) {
                    yield new ItemStack(randomWeapon(random));
                }
                yield random.nextFloat() < 0.55F
                        ? new ItemStack(Elysium.ELYSIUM_REFORGE.get())
                        : new ItemStack(randomRune(random));
            }
            case 2 -> random.nextFloat() < 0.60F
                    ? new ItemStack(randomRune(random))
                    : new ItemStack(Elysium.NEUTRONIUM_INGOT.get());
            case 1 -> random.nextFloat() < 0.60F
                    ? new ItemStack(Elysium.NEUTRONIUM_INGOT.get())
                    : new ItemStack(rawMaterial(random));
            default -> new ItemStack(rawMaterial(random));
        };
    }

    private static Item rawMaterial(RandomSource random) {
        return random.nextBoolean()
                ? Elysium.AETHERIUM_INGOT.get()
                : Elysium.VOIDGLASS_INGOT.get();
    }

    private static Item randomRune(RandomSource random) {
        Item[] runes = {
                Elysium.VOIDWARD_RUNE.get(), Elysium.PLASMAFORGE_RUNE.get(),
                Elysium.NEURALSPIKE_RUNE.get(), Elysium.DIMENSIONALSHIFT_RUNE.get(),
                Elysium.KINETICSURGE_RUNE.get(), Elysium.STABILIZER_RUNE.get(),
                Elysium.REFLEX_RUNE.get(), Elysium.BARRIER_RUNE.get(),
                Elysium.PLASMA_CORE_RUNE.get(),
        };
        return runes[random.nextInt(runes.length)];
    }

    private static Item randomWeapon(RandomSource random) {
        Item[] weapons = {
                Elysium.VOIDCUT_BLADE.get(), Elysium.PLASMA_BRAND.get(),
                Elysium.NEURAL_LASH.get(), Elysium.RIFT_EDGE.get(),
                Elysium.KINETIC_MAUL.get(),
        };
        return weapons[random.nextInt(weapons.length)];
    }

    private static void addDrop(LivingDropsEvent event, LivingEntity victim,
                                Level level, ItemStack stack) {
        if (stack.isEmpty()) {
            return;
        }
        event.getDrops().add(new ItemEntity(level,
                victim.getX(), victim.getY(), victim.getZ(), stack));
    }
}
