package com.elysium.stats;

import com.elysium.element.ElysiumElement;
import com.elysium.item.ElysiumArmorItem;
import com.elysium.item.ElysiumGearData;
import com.elysium.item.ElysiumRarities;
import com.elysium.item.ElysiumSocketable;
import com.elysium.item.ElysiumSockets;
import net.minecraft.world.item.ItemStack;

/**
 * What a piece of gear adds to the character wearing it, and what it demands
 * in return.
 *
 * <h2>The grant</h2>
 *
 * Two sources, both of which climb without limit:
 *
 * <ul>
 *   <li><b>Tier.</b> Every piece gives {@code 1 + tier} points in each of the
 *       two stats its element governs, plus a smaller amount of Fortitude for
 *       being armour at all. Ascension raises tier forever, so this term does
 *       too.</li>
 *   <li><b>Reforge rolls.</b> The armour, health and speed a reforge produced
 *       are read as Fortitude, Vitality and Agility. Reforging was previously
 *       three attribute numbers on one item; now it is three stat numbers on
 *       the character, and the difference matters because ascension refills
 *       the charges.</li>
 * </ul>
 *
 * <h2>The demand</h2>
 *
 * Five character levels per tier. A piece a player cannot meet still equips —
 * refusing to equip is a fight with the inventory that nobody wins — but it
 * grants no stats and no rune affixes until they can. The tooltip says so in
 * red rather than leaving them to wonder why nothing happened.
 */
public final class ElysiumGearStats {

    private ElysiumGearStats() {
    }

    /**
     * The stats a stack grants, before any level check.
     *
     * @return {@link ElysiumStatBlock#EMPTY} for anything that is not Elysium
     *         gear
     */
    public static ElysiumStatBlock of(ItemStack stack) {
        if (!(stack.getItem() instanceof ElysiumSocketable gear)) {
            return ElysiumStatBlock.EMPTY;
        }

        int tier = gear.getEffectiveTier(stack);
        int weight = 1 + Math.max(0, tier);
        ElysiumStatBlock block = forElement(gear.getElement(), weight);

        // Armour is armour whatever it resonates with.
        if (stack.getItem() instanceof ElysiumArmorItem) {
            block = block.with(ElysiumStat.FORTITUDE, 1 + tier / 2);
        }

        // Reforge rolls, read as character stats rather than as three
        // attribute modifiers bolted to one item.
        ElysiumGearData data = ElysiumSockets.gearData(stack);
        if (data.armorBonus() > 0) {
            block = block.with(ElysiumStat.FORTITUDE, data.armorBonus());
        }
        if (data.healthBonus() > 0) {
            block = block.with(ElysiumStat.VITALITY, data.healthBonus());
        }
        if (data.speedBonus() > 0) {
            block = block.with(ElysiumStat.AGILITY, data.speedBonus());
        }

        return block;
    }

    /** The two stats an element governs, at the given weight. */
    private static ElysiumStatBlock forElement(ElysiumElement element, int weight) {
        return switch (element) {
            case VOID -> ElysiumStatBlock.of(
                    ElysiumStat.RESILIENCE, weight, ElysiumStat.WILLPOWER, weight);
            case PLASMA -> ElysiumStatBlock.of(
                    ElysiumStat.STRENGTH, weight, ElysiumStat.ACCURACY, weight);
            case NEURAL -> ElysiumStatBlock.of(
                    ElysiumStat.INTELLECT, weight, ElysiumStat.AGILITY, weight);
            case DIMENSIONAL -> ElysiumStatBlock.of(
                    ElysiumStat.AGILITY, weight, ElysiumStat.REFLEXES, weight);
            case KINETIC -> ElysiumStatBlock.of(
                    ElysiumStat.STRENGTH, weight, ElysiumStat.RETRIBUTION, weight);
            // Inert gear is exactly that: no affinity, so it pays in the two
            // things any well-made plate gives you.
            default -> ElysiumStatBlock.of(
                    ElysiumStat.FORTITUDE, weight, ElysiumStat.VITALITY, weight);
        };
    }

    /** Character level needed before a stack grants anything. */
    public static int requiredLevel(ItemStack stack) {
        if (!(stack.getItem() instanceof ElysiumSocketable gear)) {
            return 0;
        }
        return ElysiumRarities.getRequiredLevel(gear.getEffectiveTier(stack));
    }

    public static boolean meetsRequirement(int characterLevel, ItemStack stack) {
        return characterLevel >= requiredLevel(stack);
    }
}
