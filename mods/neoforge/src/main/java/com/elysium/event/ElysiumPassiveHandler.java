package com.elysium.event;

import com.elysium.Elysium;
import com.elysium.character.ElysiumCharacter;
import com.elysium.character.ElysiumClass;
import com.elysium.character.ElysiumRace;
import com.elysium.entity.ElysiumFaction;
import com.elysium.stats.ElysiumStats;
import net.minecraft.tags.DamageTypeTags;
import net.minecraft.world.damagesource.DamageSource;
import net.minecraft.world.entity.Entity;
import net.minecraft.world.entity.LivingEntity;
import net.minecraft.world.entity.player.Player;
import net.neoforged.bus.api.SubscribeEvent;
import net.neoforged.fml.common.EventBusSubscriber;
import net.neoforged.neoforge.event.entity.living.LivingFallEvent;

/**
 * Everything a race or a class does that a stat cannot express.
 *
 * The division is deliberate. Stats are numbers that scale forever and answer
 * to gear; passives are rules, they do not scale with equipment, and each one
 * changes a decision rather than a number. A Druun does not have "more damage",
 * they have a reason to keep fighting while hurt.
 *
 * Everything here is a pure function of the player and the situation, called
 * from the combat and tick handlers rather than subscribing separately, so the
 * order of effects stays visible in one place instead of depending on which
 * listener happened to register first. The exception is fall damage, which has
 * its own event and nothing else to interleave with.
 */
@EventBusSubscriber(modid = Elysium.MODID)
public final class ElysiumPassiveHandler {

    private ElysiumPassiveHandler() {
    }

    // ------------------------------------------------------------------
    // Offence
    // ------------------------------------------------------------------

    /**
     * A multiplier on outgoing melee damage.
     *
     * <ul>
     *   <li><b>Cold Blood</b> (Druun): up to +60% as health falls. Reptilian
     *       and militaristic — the archive's Ascendancy fights hardest when
     *       losing.</li>
     *   <li><b>Sanctioned Force</b> (Enforcer): +25% against the Unsworn. The
     *       Code is specific about who may be struck.</li>
     * </ul>
     */
    public static float attackScale(Player attacker, LivingEntity victim) {
        float scale = 1.0F;

        if (ElysiumCharacter.getRace(attacker) == ElysiumRace.DRUUN) {
            float missing = 1.0F - attacker.getHealth() / Math.max(1.0F, attacker.getMaxHealth());
            scale *= 1.0F + 0.60F * missing;
        }

        if (ElysiumCharacter.getElysiumClass(attacker) == ElysiumClass.ENFORCER
                && ElysiumFaction.of(victim) == ElysiumFaction.UNSWORN) {
            scale *= 1.25F;
        }

        return scale;
    }

    /**
     * What a critical hit is worth. A Marksman does not crit more often than
     * Accuracy says — they simply make it count.
     */
    public static float critMultiplier(Player attacker) {
        return ElysiumCharacter.getElysiumClass(attacker) == ElysiumClass.MARKSMAN ? 2.25F : 1.5F;
    }

    // ------------------------------------------------------------------
    // Defence
    // ------------------------------------------------------------------

    /**
     * The level at which an Imperial's racial reflection reaches half.
     *
     * It approaches 100% and never arrives: 9% at level 10, 33% at 50, 50% at
     * 100, 67% at 200, 80% at 400, 90% at 900. Diminishing the whole way, so
     * every level helps and no level finishes the job — and since character
     * level is uncapped, neither is the climb.
     */
    private static final float IMPERIAL_REFLECT_HALFWAY = 100.0F;

    /**
     * A multiplier on incoming damage, after Resilience.
     *
     * <b>Photonic</b> (Lumari): a body that has "transcended traditional
     * biology" is hard to burn and easy to hit. Elemental harm — fire and
     * blasts — is blunted by a third; ordinary physical force lands 15%
     * harder. It is a trade, not a bonus.
     */
    public static float defenceScale(Player defender, DamageSource source) {
        if (ElysiumCharacter.getRace(defender) != ElysiumRace.LUMARI) {
            return 1.0F;
        }
        boolean elemental = source.is(DamageTypeTags.IS_FIRE)
                || source.is(DamageTypeTags.IS_EXPLOSION);
        return elemental ? 0.66F : 1.15F;
    }

    /**
     * The total share of a blow a defender sends back.
     *
     * Three sources, combined through {@link ElysiumStats#combine} rather than
     * added:
     *
     * <ul>
     *   <li><b>Retribution</b>, the stat, approaching 80%.</li>
     *   <li><b>Sanctioned Answer</b> (Imperial): the racial passive, and the
     *       one the whole system was designed around. An Imperial reflects a
     *       share of every blow with no gear and no stat investment at all,
     *       and that share climbs with level toward <b>100%</b> along the same
     *       never-quite-arriving curve everything else uses — 9% at level 10,
     *       33% at 50, 50% at 100, 80% at 400. At the far end an attacker takes
     *       very nearly what they dealt, which is the Code's position on the
     *       matter stated as arithmetic.</li>
     *   <li><b>Bulwark</b> (Warden): below half health the whole share is laid
     *       over itself, so 60% becomes 84% rather than an impossible 120%.</li>
     * </ul>
     *
     * <b>Why this cannot reach 100%, and why that matters.</b> Reflecting
     * exactly all of a blow is the boundary where an attacker takes what they
     * dealt. Past it, touching an Imperial would kill you outright regardless
     * of what you hit them with, and every fight in the game would collapse
     * into the same one. The combination is bounded below 1.0 by construction,
     * so the boundary is approached and never crossed — no clamp, no special
     * case, nothing to forget to apply at the call site.
     */
    public static float totalReflectShare(Player defender) {
        float share = ElysiumStats.reflectShare(defender);

        if (ElysiumCharacter.getRace(defender) == ElysiumRace.IMPERIAL) {
            int level = ElysiumCharacter.getLevel(defender);
            share = ElysiumStats.combine(share,
                    ElysiumStats.curve(level, IMPERIAL_REFLECT_HALFWAY));
        }

        if (ElysiumCharacter.getElysiumClass(defender) == ElysiumClass.WARDEN
                && defender.getHealth() < defender.getMaxHealth() * 0.5F) {
            share = ElysiumStats.combine(share, share);
        }

        return share;
    }

    // ------------------------------------------------------------------
    // Movement
    // ------------------------------------------------------------------

    /**
     * Lightfeather and Slipstream, both of which answer the same question:
     * what happens when you come down.
     *
     * The Veylari are avian, so gravity is a formality — they ignore the first
     * ten blocks entirely and take a third of the rest. A Voidrunner is merely
     * good at landing, and halves it.
     */
    @SubscribeEvent
    public static void onFall(LivingFallEvent event) {
        if (!(event.getEntity() instanceof Player player)) {
            return;
        }

        if (ElysiumCharacter.getRace(player) == ElysiumRace.VEYLARI) {
            event.setDistance(Math.max(0.0F, event.getDistance() - 10.0F));
            event.setDamageMultiplier(event.getDamageMultiplier() * 0.33F);
            return;
        }

        if (ElysiumCharacter.getElysiumClass(player) == ElysiumClass.VOIDRUNNER) {
            event.setDamageMultiplier(event.getDamageMultiplier() * 0.5F);
        }
    }

    // ------------------------------------------------------------------
    // Standing and loot
    // ------------------------------------------------------------------

    /**
     * How much Favor a player earns, as a multiplier.
     *
     * <b>Uncounted</b> (Unsworn): half. The Empire does not reward people it
     * does not recognise, and the Unsworn start with no Presence for the same
     * reason.
     */
    public static float favorScale(Player player) {
        float scale = ElysiumStats.presenceScale(player);
        if (ElysiumCharacter.getRace(player) == ElysiumRace.UNSWORN) {
            scale *= 0.5F;
        }
        return scale;
    }

    /**
     * How much Suspicion a player earns.
     *
     * An Enforcer works for the Empire and is given the benefit of the doubt;
     * the Unsworn are not being watched closely enough to accumulate a file.
     */
    public static float suspicionScale(Player player) {
        float scale = ElysiumStats.presenceScale(player);
        if (ElysiumCharacter.getElysiumClass(player) == ElysiumClass.ENFORCER) {
            scale *= 0.6F;
        }
        if (ElysiumCharacter.getRace(player) == ElysiumRace.UNSWORN) {
            scale *= 0.5F;
        }
        return scale;
    }

    /** Unsworn shed Suspicion twice as fast as anyone else. */
    public static int decayRate(Player player) {
        return ElysiumCharacter.getRace(player) == ElysiumRace.UNSWORN ? 2 : 1;
    }

    /**
     * The chance of an extra roll on an Elysium drop: the Luck stat, plus a
     * Factor's trade.
     */
    public static float extraDropChance(Player player) {
        float chance = ElysiumStats.luckChance(player);
        if (ElysiumCharacter.getElysiumClass(player) == ElysiumClass.FACTOR) {
            // Combined rather than added, for the same reason reflection is:
            // a Factor at high Luck was previously pinned against a clamp,
            // which made the last stretch of the Luck curve worthless to
            // exactly the class built around it.
            chance = ElysiumStats.combine(chance, 0.25F);
        }
        return chance;
    }

    // ------------------------------------------------------------------
    // Work
    // ------------------------------------------------------------------

    /** An Artificer's gear wears at two thirds the rate. */
    public static boolean savesDurability(Player player) {
        return ElysiumCharacter.getElysiumClass(player) == ElysiumClass.ARTIFICER
                && player.getRandom().nextFloat() < 0.34F;
    }

    /** A Reclaimer sometimes gets a second ingot out of a vein. */
    public static boolean doublesOre(Player player) {
        return ElysiumCharacter.getElysiumClass(player) == ElysiumClass.RECLAIMER
                && player.getRandom().nextFloat() < 0.25F;
    }

    /** Reforge quality: Presence for everyone, and an Artificer's training. */
    public static float reforgeScale(Player player) {
        float scale = ElysiumStats.presenceScale(player);
        if (ElysiumCharacter.getElysiumClass(player) == ElysiumClass.ARTIFICER) {
            scale *= 1.5F;
        }
        return scale;
    }

    /** A Psion counts an aligned rune twice over. */
    public static boolean doublesAlignment(Entity entity) {
        return entity instanceof Player player
                && ElysiumCharacter.getElysiumClass(player) == ElysiumClass.PSION;
    }
}
