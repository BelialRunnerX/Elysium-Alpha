package com.elysium.stats;

import com.elysium.character.ElysiumCharacter;
import com.elysium.character.ElysiumClass;
import com.elysium.character.ElysiumRace;
import net.minecraft.world.entity.player.Player;
import net.minecraft.world.item.ItemStack;

/**
 * Where a character's numbers come from, and what they actually do.
 *
 * <h2>The sum</h2>
 *
 * <pre>
 *   total = race base
 *         + (race growth + class growth) x (level - 1)
 *         + points spent by hand
 *         + every equipped piece the character is high enough level to use
 * </pre>
 *
 * Nothing is cached. A stat total is a handful of map lookups and the things
 * that ask for one — a damage event, a second-long tick — are not hot enough
 * to justify an invalidation bug.
 *
 * <h2>The curves</h2>
 *
 * Flat stats are read straight. Proportional stats go through
 * {@code v / (v + K)}, which rises fast at first and then flattens, so:
 *
 * <ul>
 *   <li>no proportional stat can reach 100%, at any value, ever;</li>
 *   <li>every point still helps, so nothing is wasted;</li>
 *   <li>gear can grant arbitrarily many points without breaking the game.</li>
 * </ul>
 *
 * K is the value at which a stat reaches half its theoretical maximum, which
 * is the only number worth tuning per stat.
 */
public final class ElysiumStats {

    private ElysiumStats() {
    }

    // Half-way points for the proportional stats. Bigger K = slower curve.
    private static final float K_RESILIENCE = 120.0F;
    private static final float K_ACCURACY = 220.0F;
    private static final float K_REFLEXES = 380.0F;
    private static final float K_RETRIBUTION = 260.0F;
    private static final float K_LUCK = 160.0F;
    private static final float K_AGILITY = 200.0F;

    // Ceilings the curves approach but never touch.
    private static final float MAX_DODGE = 0.50F;
    private static final float MAX_CRIT = 0.75F;
    private static final float MAX_SPEED = 0.60F;
    private static final float MAX_REFLECT = 0.80F;
    private static final float MAX_LUCK = 0.90F;

    // ------------------------------------------------------------------
    // The sum
    // ------------------------------------------------------------------

    /** Everything a character has, gear included. */
    public static ElysiumStatBlock total(Player player) {
        return innate(player).plus(fromGear(player));
    }

    /** Race, class, level and spent points — everything but the gear. */
    public static ElysiumStatBlock innate(Player player) {
        ElysiumRace race = ElysiumCharacter.getRace(player);
        ElysiumClass job = ElysiumCharacter.getElysiumClass(player);
        int levels = Math.max(0, ElysiumCharacter.getLevel(player) - 1);

        ElysiumStatBlock block = race == null ? ElysiumStatBlock.EMPTY : race.getBaseStats();
        ElysiumStatBlock growth = race == null ? ElysiumStatBlock.EMPTY : race.getGrowth();
        if (job != null) {
            growth = growth.plus(job.getGrowth());
        }

        return block.plus(growth.times(levels)).plus(ElysiumCharacter.getSpent(player));
    }

    /**
     * What the equipped set is granting.
     *
     * A piece the character is too low a level for contributes nothing. It is
     * still worn, still visible, still takes damage — it simply does not pay
     * out until they have earned it.
     */
    public static ElysiumStatBlock fromGear(Player player) {
        int level = ElysiumCharacter.getLevel(player);
        ElysiumStatBlock block = ElysiumStatBlock.EMPTY;

        for (ItemStack stack : player.getArmorSlots()) {
            if (ElysiumGearStats.meetsRequirement(level, stack)) {
                block = block.plus(ElysiumGearStats.of(stack));
            }
        }

        ItemStack held = player.getMainHandItem();
        if (ElysiumGearStats.meetsRequirement(level, held)) {
            block = block.plus(ElysiumGearStats.of(held));
        }

        return block;
    }

    public static int get(Player player, ElysiumStat stat) {
        return total(player).get(stat);
    }

    // ------------------------------------------------------------------
    // The curve
    // ------------------------------------------------------------------

    /**
     * {@code value / (value + halfway)}, clamped at zero.
     *
     * The one function every proportional stat is built on. It is monotonic,
     * it is bounded by 1.0 from below, and it never divides by zero because
     * halfway is always positive.
     */
    public static float curve(int value, float halfway) {
        if (value <= 0) {
            return 0.0F;
        }
        return value / (value + halfway);
    }

    /**
     * Stacks two proportional effects the way overlapping shields stack.
     *
     * <pre>{@code   1 - (1 - a)(1 - b)   }</pre>
     *
     * Adding shares together and clamping is the obvious thing and the wrong
     * one: two 60% sources become 100% and the clamp swallows everything
     * beyond, so past a certain point more of a stat buys literally nothing
     * and the careful curves upstream stop mattering. Combining what each one
     * <em>lets through</em> instead means 60% and 60% is 84%, a third 60% makes
     * it 94%, and the total climbs toward 1.0 without ever arriving — which is
     * the same shape as every individual stat, for the same reason.
     *
     * Both inputs are clamped below 1.0, so the result is strictly less than
     * 1.0 however many times this is applied. Nothing downstream needs its own
     * ceiling.
     */
    public static float combine(float a, float b) {
        return 1.0F - (1.0F - clampShare(a)) * (1.0F - clampShare(b));
    }

    /** A proportional share, forced into [0, 1). */
    public static float clampShare(float value) {
        return Math.max(0.0F, Math.min(0.999F, value));
    }

    // ------------------------------------------------------------------
    // Derived values — what each stat is for
    // ------------------------------------------------------------------

    /** Half-hearts restored per regeneration tick. */
    public static float regenPerTick(Player player) {
        return 0.25F + get(player, ElysiumStat.VITALITY) * 0.05F;
    }

    /** Extra maximum health, so Vitality is felt as well as seen. */
    public static double bonusHealth(Player player) {
        return get(player, ElysiumStat.VITALITY) * 0.2D;
    }

    /** Armour points with nothing equipped. */
    public static double baseArmour(Player player) {
        return get(player, ElysiumStat.FORTITUDE) * 0.5D;
    }

    /** Proportional damage reduction, applied after vanilla armour. */
    public static float damageReduction(Player player) {
        return curve(get(player, ElysiumStat.RESILIENCE), K_RESILIENCE);
    }

    /**
     * The player's own base damage, before any weapon multiplies it.
     *
     * This is the number the weapon multiplier acts on — see
     * {@code ElysiumCombatHandler}. A character with no Strength gets nothing
     * from a high-multiplier weapon, and a character with a great deal of it
     * hits hard with anything.
     */
    public static float baseDamage(Player player) {
        return get(player, ElysiumStat.STRENGTH) * 0.25F;
    }

    public static double speedBonus(Player player) {
        return curve(get(player, ElysiumStat.AGILITY), K_AGILITY) * MAX_SPEED;
    }

    public static float critChance(Player player) {
        return curve(get(player, ElysiumStat.ACCURACY), K_ACCURACY) * MAX_CRIT;
    }

    public static float dodgeChance(Player player) {
        return curve(get(player, ElysiumStat.REFLEXES), K_REFLEXES) * MAX_DODGE;
    }

    /**
     * The share of a blow the Retribution stat sends back.
     *
     * Caps below the racial passive on purpose: an Imperial's answer is the
     * thing that reaches all the way, and a stat anyone can buy should not
     * match a birthright.
     */
    public static float reflectShare(Player player) {
        return curve(get(player, ElysiumStat.RETRIBUTION), K_RETRIBUTION) * MAX_REFLECT;
    }

    /**
     * Multiplies elemental advantage and rune affix strength.
     *
     * A Psion adds half again on top. Resonance was originally specified as
     * "an aligned rune counts twice", which turned out to be unimplementable
     * where it mattered: rune affixes are baked into an item's attribute
     * component, and an item has no idea who is holding it. Expressing the
     * same idea as psionic potency puts it somewhere it can actually be
     * applied, and keeps it scaling with Intellect the way the class wants.
     */
    public static float psionicScale(Player player) {
        float scale = 1.0F + get(player, ElysiumStat.INTELLECT) * 0.02F;
        if (ElysiumCharacter.getElysiumClass(player) == ElysiumClass.PSION) {
            scale *= 1.5F;
        }
        return scale;
    }

    /** Absorption capacity granted by Willpower, on top of the Barrier rune. */
    public static float shieldCapacity(Player player) {
        return get(player, ElysiumStat.WILLPOWER) * 0.4F;
    }

    /** Chance of an extra roll on an Elysium drop. */
    public static float luckChance(Player player) {
        return curve(get(player, ElysiumStat.LUCK), K_LUCK) * MAX_LUCK;
    }

    /** Multiplies Favor and Suspicion gains, and reforge quality. */
    public static float presenceScale(Player player) {
        return 1.0F + get(player, ElysiumStat.PRESENCE) * 0.02F;
    }
}
